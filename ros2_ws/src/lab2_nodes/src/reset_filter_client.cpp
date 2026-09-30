#include <chrono>
#include <cinttypes>
#include <future>
#include <memory>
#include <thread>

#include "rcl/service_introspection.h"
#include "rclcpp/rclcpp.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

int main(int argc, char ** argv)
{
  using namespace std::chrono_literals;
  using Reset = lab2_interfaces::srv::ResetFilter;
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("reset_filter_client");
  auto client = node->create_client<Reset>("/reset_filter");
  client->configure_introspection(node->get_clock(), rclcpp::ServicesQoS(),
    RCL_SERVICE_INTROSPECTION_CONTENTS);
  if (!client->wait_for_service(5s)) {
    RCLCPP_ERROR(node->get_logger(), "[CLIENT] ResetFilter service unavailable (5 s timeout).");
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spinning([&executor]() { executor.spin(); });
  int exit_code = 1;
  try {
    RCLCPP_INFO(node->get_logger(), "[CLIENT] Sending asynchronous ResetFilter request...");
    auto future = client->async_send_request(std::make_shared<Reset::Request>());
    if (future.wait_for(2s) == std::future_status::ready) {
      const auto response = future.get();
      RCLCPP_INFO(node->get_logger(),
        "[CLIENT] Response received.\nsuccess: %s\nprevious_processed_count: %" PRIu64
        "\nprevious_last_output: %s\nmessage: %s",
        response->success ? "true" : "false", response->previous_processed_count,
        response->previous_last_output.c_str(), response->message.c_str());
      exit_code = response->success ? 0 : 1;
    } else {
      client->remove_pending_request(future);
      RCLCPP_ERROR(node->get_logger(), "[CLIENT] Timeout waiting for service response.");
    }
  } catch (const std::exception & error) {
    RCLCPP_ERROR(node->get_logger(), "[CLIENT] Request failed: %s", error.what());
  }
  executor.cancel();
  spinning.join();
  rclcpp::shutdown();
  return exit_code;
}
