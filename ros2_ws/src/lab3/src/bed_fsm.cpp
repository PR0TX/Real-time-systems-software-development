#include "bed_client.hpp"

#include <atomic>
#include <smacc2/smacc.hpp>
#include <smacc2/smacc_updatable.hpp>

namespace lab3
{
struct Inspect;
struct Clean;
struct Replace;
struct Succeeded;
struct Failed;

struct EvNext : boost::statechart::event<EvNext> {};
struct EvRetry : boost::statechart::event<EvRetry> {};
struct EvFailed : boost::statechart::event<EvFailed> {};

std::atomic<int> mission_exit_code{1};

struct BedMachine : smacc2::SmaccStateMachineBase<BedMachine, Inspect>
{
  BedMachine(my_context context, smacc2::SignalDetector * detector)
  : SmaccStateMachineBase(context, detector), detector(*detector) {}

  void onInitialize() override
  {
    try {
      season = bed_season(*getNode());
    } catch (const std::exception & error) {
      RCLCPP_ERROR(getLogger(), "%s", error.what());
    }
  }

  smacc2::SignalDetector & detector;
  std::string season;
  int attempts = 0;
};

using smacc2::Transition;

template<typename Derived>
struct BedState : smacc2::SmaccState<Derived, BedMachine>, smacc2::ISmaccUpdatable
{
  using smacc2::SmaccState<Derived, BedMachine>::SmaccState;

  void onEntry()
  {
    auto & state = static_cast<Derived &>(*this);
    auto & machine = static_cast<BedMachine &>(this->getStateMachine());
    if (machine.season.empty()) {
      machine.postEvent<EvFailed>();
      return;
    }
    call_.emplace(*this->getNode(), state.service(machine));
  }

  void update() override
  {
    if (!call_) {
      return;
    }
    const auto result = call_->poll();
    if (!result) {
      return;
    }
    call_.reset();
    auto & machine = static_cast<BedMachine &>(this->getStateMachine());
    if (*result == BedResult::ok) {
      machine.postEvent<EvNext>();
    } else if (*result == BedResult::rejected && static_cast<Derived &>(*this).retry(machine)) {
      machine.postEvent<EvRetry>();
    } else {
      machine.postEvent<EvFailed>();
    }
  }

  bool retry(BedMachine &) { return false; }

private:
  std::optional<BedCall> call_;
};

struct Inspect : BedState<Inspect>
{
  using BedState::BedState;
  using reactions = boost::mpl::list<Transition<EvNext, Clean>, Transition<EvFailed, Failed>>;
  std::string service(BedMachine &) { return "bed/inspect"; }
};

struct Clean : BedState<Clean>
{
  using BedState::BedState;
  using reactions = boost::mpl::list<
    Transition<EvNext, Replace>, Transition<EvRetry, Clean>, Transition<EvFailed, Failed>>;

  std::string service(BedMachine & machine)
  {
    ++machine.attempts;
    return "bed/clean";
  }

  bool retry(BedMachine & machine) { return machine.attempts < 3; }
};

struct Replace : BedState<Replace>
{
  using BedState::BedState;
  using reactions = boost::mpl::list<Transition<EvNext, Succeeded>, Transition<EvFailed, Failed>>;
  std::string service(BedMachine & machine) { return "bed/replace_" + machine.season; }
};

struct Succeeded : smacc2::SmaccState<Succeeded, BedMachine>
{
  using SmaccState::SmaccState;
  void onEntry()
  {
    RCLCPP_INFO(getLogger(), "Mission succeeded");
    mission_exit_code = 0;
    static_cast<BedMachine &>(getStateMachine()).detector.stop();
  }
};

struct Failed : smacc2::SmaccState<Failed, BedMachine>
{
  using SmaccState::SmaccState;
  void onEntry()
  {
    RCLCPP_ERROR(getLogger(), "Mission failed");
    static_cast<BedMachine &>(getStateMachine()).detector.stop();
  }
};
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  SmaccFifoScheduler scheduler(true);
  smacc2::SignalDetector detector(&scheduler);
  const auto machine = scheduler.create_processor<lab3::BedMachine>(&detector);
  detector.setProcessorHandle(machine);
  scheduler.initiate_processor(machine);
  boost::thread state_thread([&scheduler] { scheduler(0); });
  detector.pollingLoop();
  scheduler.destroy_processor(machine);
  scheduler.terminate();
  state_thread.join();
  rclcpp::shutdown();
  return lab3::mission_exit_code.load();
}
