#!/bin/sh
# Copyright (c) 2023 Horizon Robotics.All Rights Reserved.
#
# The material in this file is confidential and contains trade secrets
# of Horizon Robotics Inc. This is proprietary information owned by
# Horizon Robotics Inc. No part of this work may be disclosed,
# reproduced, copied, transmitted, or used in any way for any purpose,
# without the express written permission of Horizon Robotics Inc.

order_file="../../config/model/input_order/bev_ipm_4d_input_order.list"
save_path="../../../data/nuscenes_bev"

image_list_file0="${save_path}/images_0.lst"
image_list_file1="${save_path}/images_1.lst"
image_list_file2="${save_path}/images_2.lst"
image_list_file3="${save_path}/images_3.lst"
image_list_file4="${save_path}/images_4.lst"
image_list_file5="${save_path}/images_5.lst"
point_list_file="${save_path}/reference_points0.lst"
prev_point_list_file="${save_path}/prev_reference_points.lst"


if [ -f "${image_list_file0}" ]; then
    rm "${image_list_file0}"
fi
if [ -f "${image_list_file1}" ]; then
    rm "${image_list_file1}"
fi
if [ -f "${image_list_file2}" ]; then
    rm "${image_list_file2}"
fi
if [ -f "${image_list_file3}" ]; then
    rm "${image_list_file3}"
fi
if [ -f "${image_list_file4}" ]; then
    rm "${image_list_file4}"
fi
if [ -f "${image_list_file5}" ]; then
    rm "${image_list_file5}"
fi
    
if [ -f "${point_list_file}" ]; then
    rm "${point_list_file}"
fi
    
if [ -f "${prev_point_list_file}" ]; then
    rm "${prev_point_list_file}"
fi

while IFS= read -r line; do
    image_line0="${save_path}/images_0/${line}.bin"
    image_line1="${save_path}/images_1/${line}.bin"
    image_line2="${save_path}/images_2/${line}.bin"
    image_line3="${save_path}/images_3/${line}.bin"
    image_line4="${save_path}/images_4/${line}.bin"
    image_line5="${save_path}/images_5/${line}.bin"
    point_line="${save_path}/reference_points0/${line}.bin"
    prev_point_line="${save_path}/prev_points/${line}.bin"
    echo "${image_line0}" >> "${image_list_file0}"
    echo "${image_line1}" >> "${image_list_file1}"
    echo "${image_line2}" >> "${image_list_file2}"
    echo "${image_line3}" >> "${image_list_file3}"
    echo "${image_line4}" >> "${image_list_file4}"
    echo "${image_line5}" >> "${image_list_file5}"
    echo "${point_line}" >> "${point_list_file}"
    echo "${prev_point_line}" >> "${prev_point_list_file}"
done < "${order_file}"
