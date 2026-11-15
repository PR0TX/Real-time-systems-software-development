#include "joint_model.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <rclcpp/rclcpp.hpp>

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int status = 0;
  try {
    const auto node = std::make_shared<rclcpp::Node>("simulate_joint");
    const double kp = node->declare_parameter("kp", lab3::default_kp);
    const double target = node->declare_parameter("target", lab3::default_target);
    const double acceleration = node->declare_parameter(
      "max_acceleration", lab3::default_acceleration);
    const auto period_ms = node->declare_parameter<std::int64_t>("period_ms", lab3::default_period_ms);
    const auto actuation_ms = node->declare_parameter<std::int64_t>("actuation_ms", lab3::default_actuation_ms);
    const auto feedback_ms = node->declare_parameter<std::int64_t>("feedback_ms", lab3::default_feedback_ms);
    const auto duration_ms = node->declare_parameter<std::int64_t>("duration_ms", 20000);
    const bool compensate = node->declare_parameter("compensate", false);
    const auto output = node->declare_parameter<std::string>("output", "");

    lab3::JointModel joint(period_ms, acceleration, actuation_ms, feedback_ms);
    if (!std::isfinite(kp) || kp < 0.0 || !std::isfinite(target) || duration_ms <= 0 ||
      duration_ms % period_ms != 0 || output.empty())
    {
      throw std::invalid_argument(
        "set output to a CSV path, use finite kp >= 0 and target, and duration_ms divisible by period_ms");
    }
    const auto parent = std::filesystem::path(output).parent_path();
    if (!parent.empty()) {
      std::filesystem::create_directories(parent);
    }
    std::ofstream csv(output);
    csv.exceptions(std::ios::failbit | std::ios::badbit);
    csv << "time_s,position_rad,velocity_rad_s,encoder_position_rad,encoder_velocity_rad_s,"
      "requested_rad_s,applied_rad_s\n" << std::setprecision(12);

    const double horizon = compensate ?
      (actuation_ms + static_cast<double>(feedback_ms)) / 1000.0 : 0.0;
    const auto steps = duration_ms / period_ms;
    for (std::int64_t step = 0; ; ++step) {
      const auto actual = joint.actual();
      const auto measured = joint.measure();
      csv << step * period_ms / 1000.0 << ',' << actual.position << ',' << actual.velocity
          << ',' << measured.position << ',' << measured.velocity << ',';
      if (step == steps) {
        csv << ",\n";
        break;
      }
      const double command = lab3::velocity_command(measured, target, kp, horizon);
      if (!std::isfinite(command)) {
        throw std::overflow_error("controller command is not finite");
      }
      const double applied = joint.advance(command);
      csv << command << ',' << applied << '\n';
    }
    csv.close();
  } catch (const std::exception & error) {
    std::cerr << "simulate_joint: " << error.what() << '\n';
    status = 1;
  }
  rclcpp::shutdown();
  return status;
}
