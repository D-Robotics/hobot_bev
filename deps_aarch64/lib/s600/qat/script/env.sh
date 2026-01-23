#!/bin/sh
# Copyright (c) 2020 Horizon Robotics.All Rights Reserved.
#
# The material in this file is confidential and contains trade secrets
# of Horizon Robotics Inc. This is proprietary information owned by
# Horizon Robotics Inc. No part of this work may be disclosed,
# reproduced, copied, transmitted, or used in any way for any purpose,
# without the express written permission of Horizon Robotics Inc.


# define the base info

# define config base dir
config_dir=../../config

# define bin and lib info
app=../../aarch64/bin/example
lib=../../aarch64/lib
export LD_LIBRARY_PATH=${lib}:${LD_LIBRARY_PATH}
