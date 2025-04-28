English| [简体中文](./README_cn.md)

# Function Overview

BEV perception algorithm is a `BEV` multi-task model trained on the [nuscenes](https://www.nuscenes.org/nuscenes) dataset using OpenExplorer.

The algorithm takes 6 sets of image data as input, including frontal, front left, front right, rear, rear left, and rear right views. The model outputs 10 categories of objects along with their corresponding 3D detection boxes, including obstacles, various types of vehicles, traffic signs, as well as semantic segmentation of lane lines, sidewalks, and road edges.

This example uses local image data as input, performs algorithm inference using BPU, publishes images of algorithm perception results, and renders them on a PC browser.

# Supported Platforms

| Platform                         | System                               |
| ---------------------------- | --------------------------------------------- |
| RDK Ultra               | Ubuntu 20.04 (Foxy) |
| RDK S100               | Ubuntu 22.04 (Humble) |

# Bill of Materials


# Usage Instructions

## Function Installation

Run the following commands in the terminal of the RDK system for quick installation:

```bash
sudo apt update
sudo apt install -y tros-humble-hobot-bev
sudo apt install -y tros-humble-websocket
```

## Prepare for Data Backfilling

Run the following commands in the terminal of the RDK system to download and unzip the dataset:

```shell
# Board-side dataset download
cd ~
wget http://archive.d-robotics.cc/TogetheROS/data/nuscenes_bev_val/nuscenes_bev_val.tar.gz

# Unzip
mkdir -p ~/hobot_bev_data
tar -zxvf ~/nuscenes_bev_val.tar.gz -C ~/hobot_bev_data
```

## Launch Algorithm and Image Visualization

Run the following commands in the terminal of the RDK system to start the algorithm and visualization:

```shell
# Configure the tros.b humble environment
source /opt/tros/humble/setup.bash

# Start the websocket service
ln -s `ros2 pkg prefix hobot_bev`/lib/qat/ qat
ln -s ~/hobot_bev_data/nuscenes_bev_val nuscenes_bev_val

ros2 launch hobot_bev hobot_bev.launch.py
```

After successful launch, open a browser on the same network computer and visit the IP address of RDK `http://IP:8000` (where IP is the IP address of RDK) to see the real-time visualization of the algorithm:

![bev](img/bev.gif)


# Interface Description

## Topics

| Name         | Message Type                           | Description                              |
| ------------ | --------------------------------------- | ---------------------------------------- |
| /image_jpeg  | sensor_msgs/msg/Image                  | Periodically publishes image topic in jpeg format |

## Parameters

| Name                         | Parameter Value                               | Description                                 |
| ---------------------------- | --------------------------------------------- | ------------------------------------------- |
| save_image               | "True"/"False", default is "False" | Save the rendered image to the path "./render".                    |


# FAQ

By setting the parameters in the runtime configuration file, users can modify the backfilling process and the output of the perception results.

1. Obtain the path of the runtime configuration file.

On the RDK, use the following commands to query the path of the runtime configuration file:

```shell
    source /opt/tros/humble/setup.bash
    ls `ros2 pkg prefix hobot_bev`/lib/hobot_bev/config/bev_gkt_mixvargenet_multitask_nuscenes/workflow_latency.json
```

2. Control the backfilling speed.

`time_diff_ms` represents the interval time for each backfilling, and the unit is milliseconds. The default value is 200 milliseconds, that is, data is backfilled every 200 milliseconds.

```json
    "time_diff_ms": 200
```

3. Save the algorithm output and the rendering results of the images.

`enable_save_output` is the switch for saving the results, and by default, the results are not saved. `view_output_dir` represents the path for saving the results, and the default value is "./output_dir".

```json
    "enable_save_output": false,
    "view_output_dir": "./output_dir",
```

4. Set the post-processing threshold of the perception algorithm?

```json
    "score_threshold": 0.5,
``` 