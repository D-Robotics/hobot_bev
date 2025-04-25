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

#include "tros_bev/bev_node.h"
#include "tros_bev/utils.h"
#include <unistd.h>
#include <sys/stat.h>

BevNode::BevNode(int argcc, char **argvv) : Node("bev_node") {
  RCLCPP_INFO(this->get_logger(), "BevNode init");

  topic_name_ = this->declare_parameter("topic_name", topic_name_);
  save_image_ = this->declare_parameter("save_image", save_image_);
  config_file_ = this->declare_parameter("config_file", config_file_);
  glog_level_ = this->declare_parameter("glog_level", glog_level_);

  RCLCPP_WARN(this->get_logger(),
    "\n topic_name: %s \n save_image: %s \n config_file: %s \n glog_level: %d",
    topic_name_.c_str(), save_image_ ? "true" : "false", config_file_.c_str(), glog_level_);

  ros_publisher_compressed_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(
      topic_name_, 10);

  std::vector<std::string> argv = {"bin", "--config_file=" + config_file_, "--glog_level=" + std::to_string(glog_level_)};
  int argc = 3;
  char* argv_char[argc];
  for (int i = 0; i < argc; i++) {
    argv_char[i] = const_cast<char*>(argv[i].c_str());
  }
  bev_wrapper_.Init(argc, argv_char);
  bev_wrapper_.SetOutputCallback(
    std::bind(&BevNode::GetRenderImgs, this, std::placeholders::_1, std::placeholders::_2));
  bev_wrapper_.Start();
}

BevNode::~BevNode() {
  RCLCPP_INFO(this->get_logger(), "BevNode destroy");
  bev_wrapper_.Stop();
}

void BevNode::GetRenderImgs(const std::vector<cv::Mat>& imgs, std::shared_ptr<FrameInfo> frame) {
  RCLCPP_INFO(this->get_logger(),
    "Get render imgs size: %ld, frame_id: %d, duration ms infer: %.2f, postp: %.2f, prep: %.2f",
    imgs.size(),
    frame->frame_id,
    frame->infer_duration / 1000.0,
    frame->pp_duration / 1000.0,
    frame->pre_duration / 1000.0);

  // "CAM_FRONT_LEFT", "CAM_FRONT", "CAM_FRONT_RIGHT",
  // "CAM_BACK_LEFT",  "CAM_BACK",  "CAM_BACK_RIGHT"
  // bev, seg
  
  cv::Mat mat_bg;
  if (imgs.size() != 8) {
    RCLCPP_WARN(rclcpp::get_logger("hobot_bev"), "Invalid imgs size: %ld", imgs.size());
    return;
  }

  cv::Mat mat_layout;
  if (LayoutImgs(imgs, mat_layout, (save_image_ ? std::string("./render") : "")) == 0 &&
    ros_publisher_compressed_) {
    // 使用opencv的imencode接口将mat转成vector，获取图片size
    std::vector<int> param;
    std::vector<uint8_t> jpeg;
    int32_t data_len = 0;
    imencode(".jpg", mat_layout, jpeg, param);
    data_len = jpeg.size();
    sensor_msgs::msg::CompressedImage::UniquePtr msg(new sensor_msgs::msg::CompressedImage());
    msg->header.stamp = this->now();
    msg->header.frame_id = "default_cam";
    msg->format = "jpeg";
    msg->data.resize(data_len);
    memcpy(&msg->data[0], jpeg.data(), data_len);
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                "Publish ros compressed image msg, format: %s, topic: %s",
                msg->format.data(),
                topic_name_.data());
    ros_publisher_compressed_->publish(std::move(msg));
  }
}
