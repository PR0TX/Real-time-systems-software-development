FROM ros:lyrical-ros-base
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential python3-colcon-common-extensions \
    ros-lyrical-rclcpp-action ros-lyrical-rclcpp-components \
    ros-lyrical-std-msgs ros-lyrical-sensor-msgs \
    ros-lyrical-rosidl-default-generators ros-lyrical-pcl-conversions libpcl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get upgrade -y && rm -rf /var/lib/apt/lists/*
COPY ros2_ws/src /opt/lab2/ros2_ws/src
WORKDIR /home/ubuntu/ros2_lab2_colcon
RUN source /opt/ros/lyrical/setup.bash && \
    colcon --log-base log build --symlink-install \
    --base-paths /opt/lab2/ros2_ws/src --build-base build --install-base install \
    --parallel-workers 2 --cmake-args -DCMAKE_BUILD_TYPE=Release
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
