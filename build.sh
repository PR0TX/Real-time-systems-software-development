#!/usr/bin/env bash
set -eo pipefail
LAB2_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
LAB2_BUILD_ROOT="${LAB2_BUILD_ROOT:-/home/ubuntu/ros2_lab2_colcon}"
source /opt/ros/lyrical/setup.bash

mkdir -p "$LAB2_BUILD_ROOT"
cd "$LAB2_BUILD_ROOT"
colcon --log-base "$LAB2_BUILD_ROOT/log" build --symlink-install \
  --base-paths "$LAB2_DIR/ros2_ws/src" \
  --build-base "$LAB2_BUILD_ROOT/build" --install-base "$LAB2_BUILD_ROOT/install" \
  --parallel-workers 2 --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo "$@"
