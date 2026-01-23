#!/bin/sh
# Copyright (c) 2023 Horizon Robotics.All Rights Reserved.
#
# The material in this file is confidential and contains trade secrets
# of Horizon Robotics Inc. This is proprietary information owned by
# Horizon Robotics Inc. No part of this work may be disclosed,
# reproduced, copied, transmitted, or used in any way for any purpose,
# without the express written permission of Horizon Robotics Inc.

order_file="../../config/model/input_order/bevformer_input_order.list"
save_path="../../../data/nuscenes_bev"

image_folders=("images_0" "images_1" "images_2" "images_3" "images_4" "images_5" "reference_points0" "reference_points1" "reference_points2" "reference_points3" "prev_points") 

declare -A image_list_files
for folder in "${image_folders[@]}"; do
    image_list_files["$folder"]="${save_path}/${folder}.lst"
done
point_list_file="${save_path}/reference_points0.lst"
prev_point_list_file="${save_path}/prev_reference_points.lst"

for list_file in "${image_list_files[@]}"; do
    if [ -f "${list_file}" ]; then
        rm "${list_file}"
    fi
done

while IFS= read -r line; do
    for folder in "${image_folders[@]}"; do
        image_line="${save_path}/${folder}/${line}.bin"
        echo "${image_line}" >> "${image_list_files[$folder]}"
    done
done < "${order_file}"
