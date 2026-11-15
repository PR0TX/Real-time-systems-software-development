FROM ros:lyrical-ros-base

SHELL ["/bin/bash", "-c"]
WORKDIR /opt/lab3
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git libboost-thread-dev liblttng-ust-dev \
    python3-colcon-common-extensions python3-matplotlib \
    ros-lyrical-behaviortree-cpp ros-lyrical-controller-manager \
    ros-lyrical-joint-state-broadcaster ros-lyrical-robot-state-publisher \
    ros-lyrical-ros2controlcli ros-lyrical-rosidl-default-generators \
    ros-lyrical-std-srvs ros-lyrical-tracetools-launch \
    && apt-get clean && find /var/lib/apt/lists -type f -delete

COPY build.sh .
COPY ros2_ws/src ros2_ws/src
ENV LAB3_BUILD_ROOT=/opt/lab3-build
RUN bash build.sh

CMD ["bash", "-lc", "source /opt/lab3-build/install/setup.bash && exec bash"]
