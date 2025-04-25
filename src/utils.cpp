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

#include "tros_bev/utils.h"
#include <unistd.h>
#include <sys/stat.h>

int LayoutImgs(const std::vector<cv::Mat>& imgs, cv::Mat& mat_bg, std::string path) {
  // "CAM_FRONT_LEFT", "CAM_FRONT", "CAM_FRONT_RIGHT",
  // "CAM_BACK_LEFT",  "CAM_BACK",  "CAM_BACK_RIGHT"
  // bev, seg
  
  if (imgs.size() != 8) {
    printf("Invalid imgs size: %ld\n", imgs.size());
    return -1;
  }
  
  int scaled_img_width = imgs.at(0).cols;
  int scaled_img_height = imgs.at(0).rows;
  
  
  // 逆时针旋转90°使检测框渲染图片的上方是车辆前进方向
  // 顺时针90°旋转: transpose + flip(tmp,dst,1)
  // 逆时针90°旋转:transpose + flip(tmp,dst,0)
  // 180°旋转: flip(src,dst,-1)
	cv::Mat img_bev = imgs.at(6);
	// cv::transpose(imgs.at(6), img_bev);
	// 上下翻转
  cv::flip(img_bev, imgs.at(6), 0);

	cv::Mat img_seg;
  float resize_ratio = static_cast<float>(img_bev.rows) / static_cast<float>(imgs.at(7).rows);
  // RCLCPP_DEBUG(rclcpp::get_logger("hobot_bev"),
  //             "img_bev cols: %d, rows: %d, resize_ratio: %f",
  //             img_bev.cols, img_bev.rows, resize_ratio);
  cv::resize(imgs.at(7), img_seg,
    cv::Size(imgs.at(7).cols * resize_ratio, imgs.at(7).rows * resize_ratio));

  int bg_img_width = scaled_img_width * 3;
  int bg_img_height = scaled_img_height * 2 + std::max(img_bev.rows, img_seg.rows);

  mat_bg = cv::Mat(bg_img_height, bg_img_width, CV_8UC3, cv::Scalar::all(0));

  // RCLCPP_DEBUG(rclcpp::get_logger("hobot_bev"),
  //             "mat bg cols: %d, rows: %d",
  //             mat_bg.cols, mat_bg.rows);

  int copy_offset_w = 0;
  int copy_offset_h = 0;

  for (size_t idx = 0; idx < imgs.size(); idx++) {
    const cv::Mat* mat_tmp = &imgs.at(idx);
    if (idx == 6) {
      mat_tmp = &img_bev;
    } else if (idx == 7) {
      mat_tmp = &img_seg;
    }

    // RCLCPP_DEBUG(rclcpp::get_logger("hobot_bev"),
    //             "img [%ld] cols: %d, rows: %d, copy_offset_w: %d, copy_offset_h: %d",
    //             idx, mat_tmp->cols, mat_tmp->rows,
    //             copy_offset_w, copy_offset_h);
    // cv::resize(bgr_mat, mat_tmp, cv::Size(scaled_img_width, scaled_img_height));
    // render img info
    // cv::putText(mat_tmp,
    //             render_para.image_infos.at(idx),
    //             cv::Point2f(10, scaled_img_height - 10),
    //             cv::HersheyFonts::FONT_HERSHEY_SIMPLEX,
    //             0.5,
    //             cv::Scalar(255, 255, 255),
    //             1.5);

    mat_tmp->copyTo(mat_bg(cv::Rect(copy_offset_w, copy_offset_h,
                                  mat_tmp->cols,
                                  mat_tmp->rows)));
    copy_offset_w += scaled_img_width;
    if (2 == idx || 5 == idx) {
      copy_offset_w = 0;
      copy_offset_h += scaled_img_height;
    }
  }

  if (!path.empty()) {
    static int count = 0;
    if (access(path.c_str(), 0) != 0) {
      printf("mkdir %s\n", path.c_str());
      mkdir(path.c_str(), 0777);
    }
    // std::string saving_path = path + "/render_" + std::to_string(frame->frame_id) + ".jpeg";
    std::string saving_path = path + "/render_" + std::to_string(count++) + ".jpeg";
    printf("Save result to file: %s\n",
      saving_path.c_str());
    cv::imwrite(saving_path, mat_bg);
  }

  return 0;
}