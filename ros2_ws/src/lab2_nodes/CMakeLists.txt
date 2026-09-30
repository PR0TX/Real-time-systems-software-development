cmake_minimum_required(VERSION 3.16)
project(lab2_nodes)
# ROS 2 Lyrical потребують(C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
find_package(ament_cmake REQUIRED)
find_package(rcl REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(lab2_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED COMPONENTS common)
find_package(Threads REQUIRED)

foreach(node data_filter reset_filter_client array_action_server)
  add_executable(${node} src/${node}.cpp)
  target_link_libraries(${node} PRIVATE rcl::rcl rclcpp::rclcpp
    rclcpp_action::rclcpp_action lab2_interfaces::lab2_interfaces__rosidl_typesupport_cpp
    std_msgs::std_msgs__rosidl_typesupport_cpp Threads::Threads)
  install(TARGETS ${node} DESTINATION lib/${PROJECT_NAME})
endforeach()

add_library(pointcloud_components SHARED src/pointcloud_components.cpp)
target_include_directories(pointcloud_components PRIVATE include ${PCL_INCLUDE_DIRS})
target_link_libraries(pointcloud_components PRIVATE rclcpp::rclcpp
  rclcpp_components::component sensor_msgs::sensor_msgs__rosidl_typesupport_cpp
  pcl_conversions::pcl_conversions ${PCL_LIBRARIES})
rclcpp_components_register_nodes(pointcloud_components
  "lab2_nodes::PointCloudPublisherComponent"
  "lab2_nodes::PointCloudSubscriberComponent")
install(TARGETS pointcloud_components LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME})

ament_package()
