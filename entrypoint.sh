#!/usr/bin/env bash

set -eo pipefail

source "/opt/ros/${ROS_DISTRO}/setup.bash"

if [[ -f "/ros2_ws/install/setup.bash" ]]; then
  source "/ros2_ws/install/setup.bash"
fi

exec "$@"