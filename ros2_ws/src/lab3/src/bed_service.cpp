#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>

#include <cstdint>

class BedService : public rclcpp::Node
{
  using Trigger = std_srvs::srv::Trigger;

public:
  BedService() : Node("bed_service")
  {
    pillows_ = declare_parameter("has_pillows", true);
    sheets_ = declare_parameter("has_sheets", true);
    clean_on_attempt_ = declare_parameter<std::int64_t>("clean_on_attempt", 2);
    if (clean_on_attempt_ < 0 || clean_on_attempt_ > 3) {
      throw std::invalid_argument("clean_on_attempt must be in [0, 3]; 0 means a persistent stain");
    }

    inspect_ = create_service<Trigger>(
      "bed/inspect", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr result) {
        result->success = true;
        result->message = std::string("pillows=") + (pillows_ ? "present" : "missing") +
          " sheets=" + (sheets_ ? "present" : "missing");
        RCLCPP_INFO(get_logger(), "Inspection: %s", result->message.c_str());
      });
    clean_ = create_service<Trigger>(
      "bed/clean", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr result) {
        if (!cleaned_ && attempts_ < 3) {
          ++attempts_;
          cleaned_ = attempts_ == clean_on_attempt_;
          RCLCPP_INFO(get_logger(), "Cleaning attempt %d: %s", attempts_,
            cleaned_ ? "clean" : "stain remains");
        }
        result->success = cleaned_;
        result->message = cleaned_ ? "clean" : "stain remains";
      });
    summer_ = replacement_service("summer", "light");
    winter_ = replacement_service("winter", "warm");
  }

private:
  rclcpp::Service<Trigger>::SharedPtr replacement_service(
    const std::string & season, const std::string & blanket)
  {
    return create_service<Trigger>("bed/replace_" + season,
      [this, blanket](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr result) {
        result->success = cleaned_;
        if (!cleaned_) {
          result->message = "bed must be clean before replacing linen";
          return;
        }
        pillows_ = true;
        sheets_ = true;
        result->message = "fresh linen, pillows and " + blanket + " blanket installed";
        RCLCPP_INFO(get_logger(), "Replacement: %s", result->message.c_str());
      });
  }

  bool pillows_;
  bool sheets_;
  bool cleaned_ = false;
  std::int64_t clean_on_attempt_;
  int attempts_ = 0;
  rclcpp::Service<Trigger>::SharedPtr inspect_, clean_, summer_, winter_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<BedService>());
  } catch (const std::exception & error) {
    RCLCPP_ERROR(rclcpp::get_logger("bed_service"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
