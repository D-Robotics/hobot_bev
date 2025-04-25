# Copyright (c) 2024，D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import time

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch_ros.actions import Node
from launch.substitutions import TextSubstitution
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python import get_package_share_directory
from ament_index_python.packages import get_package_prefix

def generate_launch_description():
    pkg_path = os.path.join(
        get_package_prefix('hobot_bev'),
        "lib/hobot_bev")
    print("hobot_bev path is ", pkg_path)

    # args that can be set from the command line or a default will be used
    save_image_launch_arg = DeclareLaunchArgument(
        "save_image", default_value=TextSubstitution(text="False")
    )
    config_file_launch_arg = DeclareLaunchArgument(
        "config_file", default_value=TextSubstitution(text=pkg_path+"/config/bev_gkt_mixvargenet_multitask_nuscenes/workflow_latency.json")
    )
    glog_level_launch_arg = DeclareLaunchArgument(
        "glog_level", default_value=TextSubstitution(text="1")
    )

    log_level_launch_arg = DeclareLaunchArgument(
        "log_level", default_value=TextSubstitution(text="info")
    )

    # ros2 run hobot_bev hobot_bev --config_file=`ros2 pkg prefix hobot_bev`/lib/hobot_bev/config/bev_gkt_mixvargenet_multitask_nuscenes/workflow_latency.json
    hobot_bev_node = Node(
        package='hobot_bev',
        executable='hobot_bev',
        output='screen',
        parameters=[
            {"save_image": LaunchConfiguration('save_image')},
            {"config_file": LaunchConfiguration('config_file')},
            {"glog_level": LaunchConfiguration('glog_level')}
        ],
        arguments=['--ros-args', '--log-level', LaunchConfiguration('log_level')]
    )

    web_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('websocket'),
                'launch/websocket.launch.py')),
        launch_arguments={
            'websocket_only_show_image': 'True',
            'image_topic': 'image_jpeg'
        }.items()
    )
    
    return LaunchDescription([
        save_image_launch_arg,
        config_file_launch_arg,
        glog_level_launch_arg,
        log_level_launch_arg,
        hobot_bev_node,
        web_node
    ])
