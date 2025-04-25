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

#include <rclcpp/rclcpp.hpp>
#include "tros_bev/utils.h"
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    RCLCPP_ERROR(rclcpp::get_logger("layout_imgs"), "Please int imgs path");
    return -1;
  }

  rclcpp::init(argc, argv);

  std::string path = argv[1];
  std::vector<std::string> sub_paths = {
    "CAM_FRONT_LEFT", "CAM_FRONT", "CAM_FRONT_RIGHT",
    "CAM_BACK_LEFT",  "CAM_BACK",  "CAM_BACK_RIGHT",
    "BEV", "SEG"
  };

  // 0.png
  int img_frameid = 0;
  while (rclcpp::ok()) {
    std::string img_name = std::to_string(img_frameid) + ".png";
    std::vector<cv::Mat> imgs;
    for (auto &sub_path : sub_paths) {
      std::string img_fname = path + "/" + sub_path + "/" + img_name;
      if (access(img_fname.data(), 0) != 0) {
        RCLCPP_ERROR(rclcpp::get_logger("layout_imgs"),
                      "image file %s not exist", img_fname.c_str());
        break;
      }
      cv::Mat img = cv::imread(img_fname);
      imgs.push_back(img);
    }

    if (imgs.size() != 8) {
      RCLCPP_WARN(rclcpp::get_logger("layout_imgs"), "Invalid imgs size: %ld", imgs.size());
      break;
    }
    cv::Mat mat_layout;
    LayoutImgs(imgs, mat_layout, std::string("./render_layout"));
    
    img_frameid++;
  }

  rclcpp::shutdown();

  return 0;
}
