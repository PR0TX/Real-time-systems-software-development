#!/usr/bin/env bash
set -eo pipefail

lab_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_root="${LAB3_BUILD_ROOT:-$HOME/ros2_lab3_colcon}"
source /opt/ros/lyrical/setup.bash

mkdir -p "$build_root"
cd "$build_root"
build_root="$(pwd -P)"


smacc="$build_root/deps/SMACC2"
revision=eaf2a6fb6799b1426fd77e88bc8eb610cc3c295a
if [[ ! -f "$smacc/.lab3-lyrical" ]]; then
  if [[ ! -d "$smacc/.git" ]]; then
    mkdir -p "$smacc"
    git -C "$smacc" init --quiet
    git -C "$smacc" remote add origin https://github.com/robosoft-ai/SMACC2.git
  fi
  git -C "$smacc" fetch --quiet --depth 1 origin "$revision"
  git -C "$smacc" checkout --quiet --detach "$revision"

  sed -i '/^ament_target_dependencies(smacc2 /d; /^target_link_libraries(smacc2$/a\
  rclcpp::rclcpp rclcpp_action::rclcpp_action tracetools::tracetools\
  smacc2_msgs::smacc2_msgs__rosidl_typesupport_cpp' "$smacc/smacc2/CMakeLists.txt"
  sed -i 's/SmaccClientBehavior \* globalreference/ISmaccClientBehavior * globalreference/' \
    "$smacc/smacc2/include/smacc2/impl/smacc_state_machine_impl.hpp"

  find "$smacc/smacc2/include" "$smacc/smacc2/src" -type f \
    \( -name '*.hpp' -o -name '*.cpp' \) -exec sed -i \
    -e 's/\bDECLARE_TRACEPOINT\b/_DECLARE_TRACEPOINT/g' \
    -e 's/\bTRACEPOINT\b/TRACETOOLS_TRACEPOINT/g' {} +
  touch "$smacc/.lab3-lyrical"
fi

export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
colcon build --merge-install --executor sequential \
  --base-paths "$lab_dir/ros2_ws/src" "$smacc/smacc2" "$smacc/smacc2_msgs" \
  --cmake-args -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 -DBUILD_TESTING=OFF \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_POLICY_DEFAULT_CMP0167=OLD "$@"
