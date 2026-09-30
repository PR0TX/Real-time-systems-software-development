#include <atomic>
#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab2_interfaces/action/process_array.hpp"

class ArrayActionServer : public rclcpp::Node
{
  using Action = lab2_interfaces::action::ProcessArray;
  using GoalHandle = rclcpp_action::ServerGoalHandle<Action>;
public:
  ArrayActionServer() : Node("array_action_server")
  {
    using namespace std::placeholders;
    server_ = rclcpp_action::create_server<Action>(this, "/process_array",
      std::bind(&ArrayActionServer::handle_goal, this, _1, _2),
      std::bind(&ArrayActionServer::handle_cancel, this, _1),
      std::bind(&ArrayActionServer::handle_accepted, this, _1));
  }
  ~ArrayActionServer() override
  {
    stopping_ = true;
    // join очікує завершення робочого потоку, перш ніж вузол буде знищено.
    if (worker_.joinable()) { worker_.join(); }
  }
private:
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &, std::shared_ptr<const Action::Goal> goal)
  {
    if (goal->data.empty() || goal->data.size() > std::numeric_limits<uint32_t>::max()) {
      RCLCPP_WARN(get_logger(), "[ACTION] Rejecting an empty or oversized array.");
      return rclcpp_action::GoalResponse::REJECT;
    }
    for (const auto value : goal->data) {
      if (value > std::numeric_limits<int32_t>::max() / 2 ||
        value < std::numeric_limits<int32_t>::min() / 2)
      {
        RCLCPP_WARN(get_logger(), "[ACTION] Rejecting int32 overflow in value * 2.");
        return rclcpp_action::GoalResponse::REJECT;
      }
    }
    // одна ціль одночасно
    if (busy_.exchange(true)) {
      RCLCPP_WARN(get_logger(), "[ACTION] Rejecting goal: server is busy.");
      return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }
  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle>)
  {
    RCLCPP_INFO(get_logger(), "[ACTION] Cancellation requested.");
    return rclcpp_action::CancelResponse::ACCEPT;
  }
  void handle_accepted(const std::shared_ptr<GoalHandle> goal)
  {
    if (worker_.joinable()) { worker_.join(); }

    worker_ = std::thread([this, goal]() { execute(goal); });
  }
  void execute(const std::shared_ptr<GoalHandle> goal)
  {
    using namespace std::chrono_literals;
    auto result = std::make_shared<Action::Result>();
    auto feedback = std::make_shared<Action::Feedback>();
    const auto & data = goal->get_goal()->data;
    feedback->total = static_cast<uint32_t>(data.size());
    RCLCPP_INFO(get_logger(), "[ACTION] Goal accepted. Elements: %zu", data.size());
    try {
      for (size_t i = 0; i < data.size(); ++i) {
        // 250 мс роботи; кожні 25 мс перевірка скасування та завершення вузла
        for (int tick = 0; tick < 10 && !goal->is_canceling() && !stopping_ && rclcpp::ok(); ++tick) {
          std::this_thread::sleep_for(25ms);
        }
        if (stopping_ || !rclcpp::ok()) { busy_ = false; return; }
        if (goal->is_canceling()) {
          result->success = false;
          result->message = "Canceled; processed_data contains the completed prefix";
          goal->canceled(result);
          RCLCPP_INFO(get_logger(), "[ACTION] Goal CANCELED.");
          busy_ = false;
          return;
        }
        // якщо завд. не визначає операцію над масивом -> output = input * 2.
        const int32_t value = data[i] * 2;
        result->processed_data.push_back(value);
        feedback->current_index = static_cast<uint32_t>(i + 1);
        feedback->current_value = value;
        feedback->progress_percent = 100.0F * feedback->current_index / feedback->total;
        goal->publish_feedback(feedback);
        RCLCPP_INFO(get_logger(), "[ACTION] Processing %u/%u, value=%d (%.1f%%)",
          feedback->current_index, feedback->total, value, feedback->progress_percent);
      }
      result->success = true;
      result->message = "Array processed successfully";
      goal->succeed(result);
      RCLCPP_INFO(get_logger(), "[ACTION] Goal SUCCEEDED.");
    } catch (const std::exception & error) {
      RCLCPP_ERROR(get_logger(), "[ACTION] Execution failed: %s", error.what());
      if (rclcpp::ok() && goal->is_active()) {
        result->success = false;
        result->message = error.what();
        if (goal->is_canceling()) { goal->canceled(result); }
        else { goal->abort(result); }
      }
    }
    busy_ = false;
  }
  rclcpp_action::Server<Action>::SharedPtr server_;
  std::thread worker_;
  std::atomic<bool> busy_{false};
  std::atomic<bool> stopping_{false};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ArrayActionServer>());
  rclcpp::shutdown();
  return 0;
}
