#include "bed_client.hpp"

#include <ament_index_cpp/get_package_share_path.hpp>
#include <behaviortree_cpp/bt_factory.h>

namespace
{
class BedStep : public BT::StatefulActionNode
{
public:
  BedStep(const std::string & name, const BT::NodeConfig & config, rclcpp::Node::SharedPtr node)
  : BT::StatefulActionNode(name, config), node_(std::move(node)) {}

  static BT::PortsList providedPorts()
  {
    return {BT::InputPort<std::string>("service")};
  }

  BT::NodeStatus onStart() override
  {
    call_.emplace(*node_, getInput<std::string>("service").value());
    return onRunning();
  }

  BT::NodeStatus onRunning() override
  {
    const auto result = call_->poll();
    if (!result) {
      return BT::NodeStatus::RUNNING;
    }
    call_.reset();
    if (*result == lab3::BedResult::unavailable) {
      throw std::runtime_error("bed service unavailable");
    }
    return *result == lab3::BedResult::ok ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }

  void onHalted() override { call_.reset(); }

private:
  rclcpp::Node::SharedPtr node_;
  std::optional<lab3::BedCall> call_;
};
}

int main(int argc, char ** argv)
{
  using namespace std::chrono_literals;
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("bed_bt");
  int exit_code = 1;
  try {
    auto blackboard = BT::Blackboard::create();
    blackboard->set("replacement_service", "bed/replace_" + lab3::bed_season(*node));
    BT::BehaviorTreeFactory factory;
    factory.registerNodeType<BedStep>("BedStep", node);
    const auto tree_file = ament_index_cpp::get_package_share_path("lab3") / "config" / "bed.xml";
    auto tree = factory.createTreeFromFile(tree_file, blackboard);

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    rclcpp::WallRate rate(20ms);
    while (rclcpp::ok()) {
      executor.spin_some();
      const auto status = tree.tickExactlyOnce();
      if (status != BT::NodeStatus::RUNNING) {
        exit_code = status == BT::NodeStatus::SUCCESS ? 0 : 1;
        break;
      }
      rate.sleep();
    }
    tree.haltTree();
  } catch (const std::exception & error) {
    RCLCPP_ERROR(node->get_logger(), "%s", error.what());
  }
  RCLCPP_INFO(node->get_logger(), "Mission %s", exit_code == 0 ? "succeeded" : "failed");
  rclcpp::shutdown();
  return exit_code;
}
