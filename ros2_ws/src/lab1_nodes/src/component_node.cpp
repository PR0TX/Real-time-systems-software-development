#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

namespace lab1_nodes
{

class Lab1Component : public rclcpp::Node
{
public:
  explicit Lab1Component(const rclcpp::NodeOptions & options)
  : rclcpp::Node("lab1_component", options)
  {
    RCLCPP_INFO(get_logger(), "Lab1Component has been created");
  }
};

}  

RCLCPP_COMPONENTS_REGISTER_NODE(lab1_nodes::Lab1Component)

