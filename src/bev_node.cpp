// Copyright (c) 2024，D-Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <string>

#include "bev_node.h"

namespace hobot {
namespace bev {

BevNode::BevNode(const std::string& node_name,
                             const rclcpp::NodeOptions& options)
    : hobot::dnn_node::DnnNode(node_name, options) {
  this->declare_parameter<std::string>("config_file", config_file_);
  this->declare_parameter<std::string>("model_file", model_file_);
  this->declare_parameter<std::string>("pkg_path", pkg_path_);
  this->declare_parameter<std::string>("image_pre_path", image_pre_path_);

  this->get_parameter<std::string>("config_file", config_file_);
  this->get_parameter<std::string>("model_file", model_file_);
  this->get_parameter<std::string>("pkg_path", pkg_path_);
  this->get_parameter<std::string>("image_pre_path", image_pre_path_);

  RCLCPP_WARN_STREAM(rclcpp::get_logger("bev_node"),
    "\n config_file: " << config_file_
    << "\n model_file: " << model_file_
    << "\n pkg_path: " << pkg_path_
    << "\n image_pre_path: " << image_pre_path_);

  // Init中使用DNNNodeSample子类实现的SetNodePara()方法进行算法推理的初始化
  if (Init() != 0) {
    RCLCPP_ERROR(rclcpp::get_logger("bev_node"), "Node init fail!");
    rclcpp::shutdown();
    return;
  }

  // 创建消息发布者，发布算法推理消息
  msg_publisher_ = this->create_publisher<ai_msgs::msg::PerceptionTargets>(
      "/bev_node", 10);

  ros_publisher_ =
      this->create_publisher<sensor_msgs::msg::Image>(msg_pub_topic_name_, 10);
      
  sp_bev_render_ = std::make_shared<BevRender>();

  // RunSingleFeedInfer();
  RunBatchFeedInfer();
}

int BevNode::SetNodePara() {
  if (access(config_file_.c_str(), F_OK) != 0) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger("hobot_bev"), "File is not exist! config_file_: " << config_file_);
    return -1;
  }
  if (access(model_file_.c_str(), F_OK) != 0) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger("hobot_bev"), "File is not exist! model_file: " << model_file_);
    return -1;
  }

  dnn_node_para_ptr_->model_file = model_file_;
  dnn_node_para_ptr_->task_num = 4;
  sp_preprocess_ = std::make_shared<PreProcess>(config_file_);
  sp_postprocess_ = std::make_shared<BevPostProcess>(config_file_);
  
  return 0;
}

// 推理结果回调，解析算法输出，通过ROS Msg发布消息
int BevNode::PostProcess(
    const std::shared_ptr<hobot::dnn_node::DnnNodeOutput>& node_output) {
  if (!rclcpp::ok()) {
    return 0;
  }

  // 后处理开始时间
  auto tp_start = std::chrono::system_clock::now();

  // 1 创建用于发布推理结果的ROS Msg
  ai_msgs::msg::PerceptionTargets::UniquePtr pub_data(
      new ai_msgs::msg::PerceptionTargets());

  // 2 将推理输出对应图片的消息头填充到ROS Msg
  pub_data->set__header(*node_output->msg_header);

  // 3 使用自定义的Parse解析方法，解析算法输出的DNNTensor类型数据
  // 3.1
  std::vector<std::shared_ptr<hobot::dnn_node::DNNTensor>>
      results;

  // 3.2 开始解析
  std::shared_ptr<HobotBevData> det_result = std::make_shared<HobotBevData>();
  sp_postprocess_->OutputsPostProcess(node_output->output_tensors, det_result);
  
  if (node_output->rt_stat) {
    auto tp_now = std::chrono::system_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                        tp_now - tp_start)
                        .count();
    RCLCPP_WARN(rclcpp::get_logger("bev_node"),
                "input fps: %.2f, out fps: %.2f, infer time ms: %d, "
                "post process time ms: %d",
                node_output->rt_stat->input_fps,
                node_output->rt_stat->output_fps,
                node_output->rt_stat->infer_time_ms,
                interval);
  }
  
  auto sp_bev_node_out = std::dynamic_pointer_cast<BevNodeOutput>(node_output);
  if (sp_bev_node_out) {
    RCLCPP_INFO(rclcpp::get_logger("bev_node"),
                "Render start");
    cv::Mat mat_bg;
    sp_bev_render_->Render(sp_bev_node_out->image_files, det_result, mat_bg);

    auto msg = sensor_msgs::msg::Image();
    msg.height = mat_bg.rows;
    msg.width = mat_bg.cols;
    msg.encoding = "jpeg";
    
    // 使用opencv的imencode接口将mat转成vector，获取图片size
    std::vector<int> param;
    std::vector<uint8_t> jpeg;
    imencode(".jpg", mat_bg, jpeg, param);
    int32_t data_len = jpeg.size();
    msg.data.resize(data_len);
    memcpy(&msg.data[0], jpeg.data(), data_len);
    struct timespec time_start = {0, 0};
    clock_gettime(CLOCK_REALTIME, &time_start);
    msg.header.stamp.sec = time_start.tv_sec;
    msg.header.stamp.nanosec = time_start.tv_nsec;
    ros_publisher_->publish(msg);
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("bev_node"),
                "Pointer cast fail");
  }

#if 0
  // 3.3 使用解析后的数据填充到ROS Msg
  for (auto& rect : results) {
    if (!rect) continue;
  }

  // 5 将算法推理输出帧率填充到ROS Msg
  if (node_output->rt_stat) {
    pub_data->set__fps(round(node_output->rt_stat->output_fps));
    // 如果算法推理统计有更新，输出算法输入和输出的帧率统计、推理耗时
    if (node_output->rt_stat->fps_updated) {
      // 后处理结束时间
      auto tp_now = std::chrono::system_clock::now();
      auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                          tp_now - tp_start)
                          .count();
      RCLCPP_WARN(rclcpp::get_logger("bev_node"),
                  "input fps: %.2f, out fps: %.2f, infer time ms: %d, "
                  "post process time ms: %d",
                  node_output->rt_stat->input_fps,
                  node_output->rt_stat->output_fps,
                  node_output->rt_stat->infer_time_ms,
                  interval);
    }
  }
  // 6 发布ROS Msg
  msg_publisher_->publish(std::move(pub_data));
#endif

  return 0;
}

void BevNode::RunSingleFeedInfer() {
  auto model = GetModel();
  if (!model) {
    RCLCPP_ERROR(rclcpp::get_logger("bev_node"), "Invalid model!");
    return;
  }
 
  auto sp_feedback_data = std::make_shared<FeedbackData>();
  for (const auto& image_file: image_files) {
    sp_feedback_data->image_files.push_back(pkg_path_ + "/" + image_file);
  }

  for (auto i = 0; i < 6; i++) {
    sp_feedback_data->points_files.push_back(pkg_path_ + "/" + "config/bev_ipm_base/" + std::to_string(i) + ".bin");
  }

  std::vector<std::shared_ptr<DNNTensor>> input_tensors;
  sp_preprocess_->CvtData2Tensors(input_tensors, model, sp_feedback_data);

  // TODO 20230420 创建tensor的时候已经添加了释放？
  // free input tensors
  // keep camera parameter tensors
  // sp_preprocess_->FreeTensors(input_tensors);

  auto dnn_output = std::make_shared<BevNodeOutput>();
  dnn_output->msg_header = std::make_shared<std_msgs::msg::Header>();
  dnn_output->image_files = sp_feedback_data->image_files;
  if (Run(input_tensors, dnn_output, true, -1, -1) < 0) {
    RCLCPP_INFO(rclcpp::get_logger("bev_node"), "Run infer fail!");
  }
}

void BevNode::RunBatchFeedInfer() {
  auto model = GetModel();
  if (!model) {
    RCLCPP_ERROR(rclcpp::get_logger("bev_node"), "Invalid model!");
    rclcpp::shutdown();
    return;
  }

  if (image_lists.size() != model_in_img_size_) {
    RCLCPP_ERROR(rclcpp::get_logger("bev_node"),
      "image_lists.size %d is unmatch with needed %d",
      image_lists.size(), model_in_img_size_);
    rclcpp::shutdown();
    return;
  }
 
  std::vector<std::vector<std::string>> image_source_lists;
  for (const auto& image_list : image_lists) {
    std::ifstream ifs(pkg_path_ + "/" + image_list);
    if (!ifs.good()) {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("bev_node"), "Open file failed: " << image_list);
      rclcpp::shutdown();
      return;
    }

    std::vector<std::string> image_source_list;
    std::string image_path;
    while (std::getline(ifs, image_path)) {
      std::string img_name = image_pre_path_ + "/" + image_path;
      if (access(img_name.c_str(), F_OK) != 0) {
        RCLCPP_ERROR_STREAM(rclcpp::get_logger("hobot_bev"), "File is not exist! img_name: " << img_name);
        rclcpp::shutdown();
        return;
      }
      image_source_list.push_back(img_name);
    }
    image_source_lists.emplace_back(image_source_list);
    ifs.close();
  }
  
  int min_img_num = image_source_lists.at(0).size();
  for (size_t idx = 0; idx < image_lists.size(); idx++) {
    RCLCPP_INFO_STREAM(rclcpp::get_logger("bev_node"),
    "image_list " << pkg_path_ + "/" + image_lists.at(idx) << " has imgs len " << image_source_lists.at(idx).size());
    if (min_img_num > image_source_lists.at(idx).size()) {
      min_img_num = image_source_lists.at(idx).size();
    }
  }

  for (int img_idx = 0; img_idx < min_img_num; img_idx++) {
    if (!rclcpp::ok()) {
      RCLCPP_INFO_STREAM(rclcpp::get_logger("bev_node"),
      "Exit batch feedback loop.");
      break;
    }
    
    RCLCPP_WARN_STREAM(rclcpp::get_logger("bev_node"),
    "loop " << img_idx << "/" << min_img_num);

    auto sp_feedback_data = std::make_shared<FeedbackData>();
    for (size_t grp_idx = 0; grp_idx < model_in_img_size_; grp_idx++) {
      std::string img_name = image_source_lists.at(grp_idx).at(img_idx);
      RCLCPP_INFO_STREAM(rclcpp::get_logger("bev_node"),
        "img_idx " << img_idx << ", grp_idx " << grp_idx << ", img_name " << img_name);
      sp_feedback_data->image_files.emplace_back(img_name);
    }
    
    for (auto i = 0; i < model_in_img_size_; i++) {
      sp_feedback_data->points_files.push_back(pkg_path_ + "/" + "config/bev_ipm_base/" + std::to_string(i) + ".bin");
    }

    std::vector<std::shared_ptr<DNNTensor>> input_tensors;
    sp_preprocess_->CvtData2Tensors(input_tensors, model, sp_feedback_data);

    auto dnn_output = std::make_shared<BevNodeOutput>();
    dnn_output->msg_header = std::make_shared<std_msgs::msg::Header>();
    dnn_output->image_files = sp_feedback_data->image_files;
    if (Run(input_tensors, dnn_output, false, -1, -1) < 0) {
      RCLCPP_INFO(rclcpp::get_logger("bev_node"), "Run infer fail!");
    }
  }
  rclcpp::shutdown();
  return;
}

}  // namespace bev
}  // namespace hobot
