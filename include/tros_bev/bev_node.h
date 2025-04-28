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

#include <fstream>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/writer.h"
#include "tros_ai_wrapper/ai_wrapper.h"

class BevNode : public rclcpp::Node {
public:
  BevNode(int argc, char **argv);

  ~BevNode();

private:
  std::string config_file_ = "";
  int glog_level_ = 1;

  std::string topic_name_ = "image_jpeg";
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr
    ros_publisher_compressed_ = nullptr;

  bool save_image_ = false;
  AIWrapper ai_wrapper_;

  void GetRenderImgs(const std::vector<cv::Mat>& imgs, std::shared_ptr<FrameInfo> frame);
};