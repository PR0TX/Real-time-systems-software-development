#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace lab3
{
inline constexpr double default_kp = 5.0;
inline constexpr double default_target = 0.05;
inline constexpr double default_acceleration = 0.5;
inline constexpr int default_period_ms = 50;
inline constexpr int default_actuation_ms = 100;
inline constexpr int default_feedback_ms = 50;

struct JointState
{
  double position = 0.0;
  double velocity = 0.0;
};

inline double velocity_command(
  JointState measured, double target, double kp, double prediction_seconds)
{
  const double predicted_position = measured.position + measured.velocity * prediction_seconds;
  return kp * (target - predicted_position);
}

// Значення, подане на кроці n, повертається на кроці n + delay_steps
template<typename T>
class DelayLine
{
public:
  explicit DelayLine(std::size_t delay_steps) : history_(delay_steps) {}

  T step(T input)
  {
    if (history_.empty()) {
      return input;
    }
    const T output = history_[next_];
    history_[next_] = input;
    next_ = (next_ + 1) % history_.size();
    return output;
  }

  void reset()
  {
    std::fill(history_.begin(), history_.end(), T{});
    next_ = 0;
  }

private:
  std::vector<T> history_;
  std::size_t next_ = 0;
};

class JointModel
{
public:
  JointModel(
    std::int64_t period_ms, double acceleration, std::int64_t actuation_ms,
    std::int64_t feedback_ms)
  : dt_(period_ms / 1000.0), acceleration_(acceleration),
    actuation_(delay_steps(actuation_ms, period_ms)),
    feedback_(delay_steps(feedback_ms, period_ms))
  {
    if (!std::isfinite(acceleration) || acceleration <= 0.0) {
      throw std::invalid_argument("max_acceleration must be finite and positive");
    }
  }

  JointState actual() const { return state_; }
  JointState measure() { return feedback_.step(state_); }

  // Інтегрування зміни швидкості й рух зі сталою швидкістю після досягнення команди
  double advance(double requested_velocity)
  {
    const double applied_velocity = actuation_.step(requested_velocity);
    const double difference = applied_velocity - state_.velocity;
    const double ramp_time = std::min(std::abs(difference) / acceleration_, dt_);
    const double velocity_change = std::clamp(
      difference, -acceleration_ * dt_, acceleration_ * dt_);
    const double next_velocity = state_.velocity + velocity_change;
    state_.position += 0.5 * (state_.velocity + next_velocity) * ramp_time +
      next_velocity * (dt_ - ramp_time);
    state_.velocity = next_velocity;
    return applied_velocity;
  }

  void reset_commands() { actuation_.reset(); }

private:
  static std::size_t delay_steps(std::int64_t delay_ms, std::int64_t period_ms)
  {
    if (period_ms <= 0 || delay_ms < 0 || delay_ms % period_ms != 0) {
      throw std::invalid_argument("delays must be non-negative multiples of a positive period_ms");
    }
    return static_cast<std::size_t>(delay_ms / period_ms);
  }

  double dt_;
  double acceleration_;
  DelayLine<double> actuation_;
  DelayLine<JointState> feedback_;
  JointState state_;
};
} 
