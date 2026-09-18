#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

class ManagedNodeA : public rclcpp_lifecycle::LifecycleNode
{
public:
  using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  ManagedNodeA()
  : rclcpp_lifecycle::LifecycleNode("managed_node_a")
  {
    RCLCPP_INFO(get_logger(), "ManagedNodeA created: state = unconfigured");
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "ManagedNodeA: on_configure()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "ManagedNodeA: on_activate()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "ManagedNodeA: on_deactivate()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "ManagedNodeA: on_cleanup()");
    return CallbackReturn::SUCCESS;
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<ManagedNodeA>();
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());
  executor.spin();

  rclcpp::shutdown();
  return 0;
}

