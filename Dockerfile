FROM ros:lyrical

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /ros2_ws

COPY ros2_ws/src /ros2_ws/src

RUN apt-get update \
    && rosdep update \
    && rosdep install --from-paths src --ignore-src -r -y \
    && rm -rf /var/lib/apt/lists/*

RUN source /opt/ros/${ROS_DISTRO}/setup.bash \
    && colcon build --symlink-install

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]

