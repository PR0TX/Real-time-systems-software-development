#include "joint_model.hpp"

#include <controller_interface/controller_interface.hpp>
#include <hardware_interface/system_interface.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <limits>

namespace lab3
{
constexpr auto position = "joint/position";
constexpr auto velocity = "joint/velocity";
constexpr double no_command = std::numeric_limits<double>::quiet_NaN();

class SimulatedJoint : public hardware_interface::SystemInterface
{
public:
  hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    position_state_ = get_state_interface_handle(position);
    velocity_state_ = get_state_interface_handle(velocity);
    velocity_command_ = get_command_interface_handle(velocity);
    return hardware_interface::CallbackReturn::SUCCESS;
  }

  hardware_interface::return_type read(const rclcpp::Time &, const rclcpp::Duration &) override
  {
    const auto measured = model_.measure();
    const bool position_written = set_state(position_state_, measured.position, false);
    const bool velocity_written = set_state(velocity_state_, measured.velocity, false);
    return position_written && velocity_written ?
      hardware_interface::return_type::OK : hardware_interface::return_type::ERROR;
  }

  hardware_interface::return_type write(const rclcpp::Time &, const rclcpp::Duration &) override
  {
    double requested = no_command;
    if (!get_command(velocity_command_, requested, false)) {
      return hardware_interface::return_type::ERROR;
    }
    if (std::isfinite(requested)) {
      model_.advance(requested);
    } else {
      // Після деактивації контролера очищення черги команд і стоп
      model_.reset_commands();
      model_.advance(0.0);
    }
    return hardware_interface::return_type::OK;
  }

private:
  hardware_interface::StateInterface::SharedPtr position_state_;
  hardware_interface::StateInterface::SharedPtr velocity_state_;
  hardware_interface::CommandInterface::SharedPtr velocity_command_;

  // Один цикл читання, оновлення та запису(крок симуляції 50 мс)
  JointModel model_{default_period_ms, default_acceleration,
    default_actuation_ms, default_feedback_ms};
};

class PositionP : public controller_interface::ControllerInterface
{
public:
  controller_interface::CallbackReturn on_init() override
  {
    try {
      auto_declare("kp", default_kp);
      auto_declare("target", default_target);
      auto_declare("compensate", false);
    } catch (const std::exception & error) {
      RCLCPP_ERROR(get_node()->get_logger(), "%s", error.what());
      return controller_interface::CallbackReturn::ERROR;
    }
    return controller_interface::CallbackReturn::SUCCESS;
  }

  controller_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    kp_ = get_node()->get_parameter("kp").as_double();
    target_ = get_node()->get_parameter("target").as_double();
    prediction_seconds_ = get_node()->get_parameter("compensate").as_bool() ?
      (default_actuation_ms + default_feedback_ms) / 1000.0 : 0.0;
    if (!std::isfinite(kp_) || kp_ < 0.0 || !std::isfinite(target_)) {
      RCLCPP_ERROR(get_node()->get_logger(), "Use finite kp >= 0 and target");
      return controller_interface::CallbackReturn::ERROR;
    }
    return controller_interface::CallbackReturn::SUCCESS;
  }

  controller_interface::InterfaceConfiguration command_interface_configuration() const override
  {
    return {controller_interface::interface_configuration_type::INDIVIDUAL, {velocity}};
  }

  controller_interface::InterfaceConfiguration state_interface_configuration() const override
  {
    return {controller_interface::interface_configuration_type::INDIVIDUAL, {position, velocity}};
  }

  controller_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    return controller_interface::CallbackReturn::SUCCESS;
  }

  controller_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    return command_interfaces_[0].set_value(no_command) ?
      controller_interface::CallbackReturn::SUCCESS : controller_interface::CallbackReturn::ERROR;
  }

  controller_interface::return_type update(const rclcpp::Time &, const rclcpp::Duration &) override
  {
    const auto measured_position = state_interfaces_[0].get_optional<double>();
    const auto measured_velocity = state_interfaces_[1].get_optional<double>();
    if (!measured_position || !measured_velocity ||
      !std::isfinite(*measured_position) || !std::isfinite(*measured_velocity))
    {
      return controller_interface::return_type::ERROR;
    }

    const double command = velocity_command(
      {*measured_position, *measured_velocity}, target_, kp_, prediction_seconds_);
    if (!std::isfinite(command)) {
      return controller_interface::return_type::ERROR;
    }
    return command_interfaces_[0].set_value(command) ?
      controller_interface::return_type::OK : controller_interface::return_type::ERROR;
  }

private:
  double kp_ = default_kp;
  double target_ = default_target;
  double prediction_seconds_ = 0.0;
};
} 

PLUGINLIB_EXPORT_CLASS(lab3::SimulatedJoint, hardware_interface::SystemInterface)
PLUGINLIB_EXPORT_CLASS(lab3::PositionP, controller_interface::ControllerInterface)
