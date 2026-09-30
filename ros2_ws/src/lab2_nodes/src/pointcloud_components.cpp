#include <chrono>
#include <memory>
#include "rclcpp_components/register_node_macro.hpp"
#include "lab2_nodes/pointcloud_type_adapter.hpp"

namespace lab2_nodes
{
class PointCloudPublisherComponent : public rclcpp::Node
{
public:
  explicit PointCloudPublisherComponent(const rclcpp::NodeOptions & options)
  : Node("pointcloud_publisher", options)
  {
  
    publisher_ = create_publisher<CloudAdapter>("/adapted_pointcloud",
      rclcpp::QoS(10).reliable().durability_volatile());
    timer_ = create_wall_timer(std::chrono::seconds(1), [this]() {
      auto cloud = std::make_unique<PclCloud>();
      cloud->header.frame_id = "map";
      for (int i = 0; i < 8; ++i) {
        cloud->push_back(pcl::PointXYZ(static_cast<float>(i), i * 0.5F, i * 0.25F));
      }
      cloud->is_dense = true;
      RCLCPP_INFO(get_logger(), "[PCL PUB] Publishing cloud, points=%zu, ptr=%p",
        cloud->size(), static_cast<void *>(cloud.get()));
      publisher_->publish(std::move(cloud));
    });
  }
private:
  rclcpp::Publisher<CloudAdapter>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

class PointCloudSubscriberComponent : public rclcpp::Node
{
public:
  explicit PointCloudSubscriberComponent(const rclcpp::NodeOptions & options)
  : Node("pointcloud_subscriber", options)
  {
    subscription_ = create_subscription<CloudAdapter>("/adapted_pointcloud",
      rclcpp::QoS(10).reliable().durability_volatile(),
      [this](std::unique_ptr<PclCloud> cloud) {
        if (cloud->empty()) {
          RCLCPP_WARN(get_logger(), "[PCL SUB] Received an empty cloud.");
          return;
        }
        const auto & first = cloud->points.front();
        const auto & last = cloud->points.back();
        RCLCPP_INFO(get_logger(),
          "[PCL SUB] Received cloud, points=%zu, ptr=%p, frame=%s, first=(%.2f,%.2f,%.2f), last=(%.2f,%.2f,%.2f)",
          cloud->size(), static_cast<void *>(cloud.get()), cloud->header.frame_id.c_str(),
          first.x, first.y, first.z, last.x, last.y, last.z);
      });
  }
private:
  rclcpp::Subscription<CloudAdapter>::SharedPtr subscription_;
};
}

RCLCPP_COMPONENTS_REGISTER_NODE(lab2_nodes::PointCloudPublisherComponent)
RCLCPP_COMPONENTS_REGISTER_NODE(lab2_nodes::PointCloudSubscriberComponent)
