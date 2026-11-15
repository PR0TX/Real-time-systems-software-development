#pragma once

#include <chrono>
#include <future>
#include <optional>
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <string>

namespace lab3
{
enum class BedResult {ok, rejected, unavailable};

inline std::string bed_season(rclcpp::Node & node)
{
  const auto season = node.declare_parameter<std::string>("season", "winter");
  if (season != "winter" && season != "summer") {
    throw std::invalid_argument("season must be winter or summer");
  }
  return season;
}

class BedCall
{
  using Clock = std::chrono::steady_clock;
  using Trigger = std_srvs::srv::Trigger;

public:
  BedCall(rclcpp::Node & node, const std::string & service)
  : client_(node.create_client<Trigger>(service)), logger_(node.get_logger()),
    deadline_(Clock::now() + std::chrono::seconds(3))
  {}

  ~BedCall()
  {
    if (request_) {
      client_->remove_pending_request(*request_);
    }
  }

  // Виконавець ROS обробляє відповіді, а опитування перевіряє готовність без очікування
  std::optional<BedResult> poll()
  {
    using namespace std::chrono_literals;
    if (request_ && request_->wait_for(0s) == std::future_status::ready) {
      const auto response = request_->get();
      request_.reset();
      RCLCPP_INFO(logger_, "%s: %s", client_->get_service_name(), response->message.c_str());
      return response->success ? BedResult::ok : BedResult::rejected;
    }
    if (Clock::now() >= deadline_) {
      RCLCPP_ERROR(logger_, "Timeout for %s", client_->get_service_name());
      return BedResult::unavailable;
    }
    if (!request_ && client_->service_is_ready()) {
      request_.emplace(client_->async_send_request(std::make_shared<Trigger::Request>()));
    }
    return std::nullopt;
  }

private:
  rclcpp::Client<Trigger>::SharedPtr client_;
  rclcpp::Logger logger_;
  Clock::time_point deadline_;
  std::optional<rclcpp::Client<Trigger>::FutureAndRequestId> request_;
};
}
