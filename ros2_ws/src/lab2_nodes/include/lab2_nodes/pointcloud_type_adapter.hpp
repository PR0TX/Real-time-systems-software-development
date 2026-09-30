#pragma once

#include <type_traits>
#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl_conversions/pcl_conversions.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/type_adapter.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

namespace lab2_nodes
{
using PclCloud = pcl::PointCloud<pcl::PointXYZ>;
}

namespace rclcpp
{
template<>
struct TypeAdapter<lab2_nodes::PclCloud, sensor_msgs::msg::PointCloud2>
{
  using is_specialized = std::true_type;
  using custom_type = lab2_nodes::PclCloud;
  using ros_message_type = sensor_msgs::msg::PointCloud2;

  static void convert_to_ros_message(const custom_type & source, ros_message_type & destination)
  {
    RCLCPP_INFO(rclcpp::get_logger("pointcloud_adapter"), "[ADAPTER] convert_to_ros_message CALLED");
    pcl::toROSMsg(source, destination);
  }
  static void convert_to_custom(const ros_message_type & source, custom_type & destination)
  {
    RCLCPP_INFO(rclcpp::get_logger("pointcloud_adapter"), "[ADAPTER] convert_to_custom CALLED");
    pcl::fromROSMsg(source, destination);
  }
};
}

namespace lab2_nodes
{
using CloudAdapter = rclcpp::TypeAdapter<PclCloud, sensor_msgs::msg::PointCloud2>;
}
