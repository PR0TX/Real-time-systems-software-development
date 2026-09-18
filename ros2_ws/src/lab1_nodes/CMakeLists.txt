cmake_minimum_required(VERSION 3.10)
project(lab1_nodes)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)

# Component Node
add_library(lab1_component SHARED
  src/component_node.cpp
)

target_compile_features(lab1_component PUBLIC cxx_std_17)

target_link_libraries(lab1_component
  PUBLIC
    rclcpp::rclcpp
    rclcpp_components::component
)

rclcpp_components_register_nodes(
  lab1_component
  "lab1_nodes::Lab1Component"
)

# Lifecycle Node A
add_executable(lifecycle_node_a
  src/lifecycle_node_a.cpp
)

target_compile_features(lifecycle_node_a PUBLIC cxx_std_17)

target_link_libraries(lifecycle_node_a
  PRIVATE
    rclcpp::rclcpp
    rclcpp_lifecycle::rclcpp_lifecycle
)

# Lifecycle Node B
add_executable(lifecycle_node_b
  src/lifecycle_node_b.cpp
)

target_compile_features(lifecycle_node_b PUBLIC cxx_std_17)

target_link_libraries(lifecycle_node_b
  PRIVATE
    rclcpp::rclcpp
    rclcpp_lifecycle::rclcpp_lifecycle
)

install(
  TARGETS lab1_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  TARGETS
    lifecycle_node_a
    lifecycle_node_b
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
