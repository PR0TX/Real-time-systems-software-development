#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <memory>
#include <string>

#include "rcl/service_introspection.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/qos_overriding_options.hpp"
#include "std_msgs/msg/string.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

class DataFilter : public rclcpp::Node
{
  using String = std_msgs::msg::String;
  using Reset = lab2_interfaces::srv::ResetFilter;
public:
  DataFilter() : Node("data_filter")
  {
    rclcpp::PublisherOptions options;

    options.event_callbacks.matched_callback =
      [this](rclcpp::MatchedInfo & info) {
        RCLCPP_INFO(get_logger(), "[LAZY] Output subscriber count = %zu", info.current_count);
        if (info.current_count > 0 && !input_) {
          input_ = create_subscription<String>("/input_topic", rclcpp::QoS(10),
            [this](String::ConstSharedPtr msg) { process(*msg); });
          RCLCPP_INFO(get_logger(), "[LAZY] Input subscription CREATED.");
        } else if (info.current_count == 0 && input_) {
          input_.reset();
          RCLCPP_INFO(get_logger(), "[LAZY] Input subscription DESTROYED.");
        }
      };
    options.qos_overriding_options = rclcpp::QosOverridingOptions({
      rclcpp::QosPolicyKind::Depth, rclcpp::QosPolicyKind::Reliability,
      rclcpp::QosPolicyKind::Durability});

    output_ = create_publisher<String>("/uppercase_topic",
      rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local(), options);
    const auto qos = output_->get_actual_qos();
    RCLCPP_INFO(get_logger(), "[QoS] depth=%zu reliability=%s durability=%s", qos.depth(),
      qos.reliability() == rclcpp::ReliabilityPolicy::Reliable ? "reliable" : "best_effort",
      qos.durability() == rclcpp::DurabilityPolicy::TransientLocal ? "transient_local" : "volatile");
    reset_ = create_service<Reset>("/reset_filter",
      [this](const Reset::Request::SharedPtr, Reset::Response::SharedPtr response) {
        response->previous_processed_count = processed_count_;
        response->previous_last_output = last_output_;
        processed_count_ = 0;
        last_output_.clear();
        response->success = true;
        response->message = "Filter state reset successfully";
        RCLCPP_INFO(get_logger(), "[SERVICE] Reset: previous count=%" PRIu64 ", output=\"%s\"",
          response->previous_processed_count, response->previous_last_output.c_str());
      });
    reset_->configure_introspection(get_clock(), rclcpp::ServicesQoS(),
      RCL_SERVICE_INTROSPECTION_CONTENTS);
  }
private:
  void process(const String & input)
  {
    String output;
    output.data = input.data;
    // std::toupper потребує unsigned char(ASCII)
    std::transform(output.data.begin(), output.data.end(), output.data.begin(),
      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    ++processed_count_;
    last_output_ = output.data;
    output_->publish(output);
    RCLCPP_INFO(get_logger(), "[FILTER] \"%s\" -> \"%s\" | processed=%" PRIu64,
      input.data.c_str(), output.data.c_str(), processed_count_);
  }
  rclcpp::Publisher<String>::SharedPtr output_;
  rclcpp::Subscription<String>::SharedPtr input_;
  rclcpp::Service<Reset>::SharedPtr reset_;
  uint64_t processed_count_{0};
  std::string last_output_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DataFilter>());
  rclcpp::shutdown();
  return 0;
}
