# Лабораторна № 3 · ROS 2 Lyrical

## Збірка

```bash
sudo apt install build-essential cmake git libboost-thread-dev liblttng-ust-dev \
  python3-colcon-common-extensions python3-matplotlib \
  ros-lyrical-behaviortree-cpp ros-lyrical-controller-manager \
  ros-lyrical-joint-state-broadcaster ros-lyrical-robot-state-publisher \
  ros-lyrical-ros2controlcli ros-lyrical-rosidl-default-generators \
  ros-lyrical-std-srvs ros-lyrical-tracetools-launch
bash build.sh
source "$HOME/ros2_lab3_colcon/install/setup.bash"
```

## 3.1 · Прибирання ліжка

```bash
ros2 launch lab3 bed.launch.py method:=fsm season:=winter has_sheets:=false
ros2 launch lab3 bed.launch.py method:=bt season:=summer has_pillows:=false clean_on_attempt:=3
ros2 launch lab3 bed.launch.py method:=bt clean_on_attempt:=0
```

## 3.2 · Експерименти та перемикання контролерів

```bash
ros2 run lab3 joint_experiments "$HOME/ros2_lab3_colcon/experiments"
ros2 launch lab3 joint_control.launch.py
```

```bash
ros2 run lab3 simulate_joint --ros-args -p kp:=5.0 -p period_ms:=50 \
  -p actuation_ms:=100 -p feedback_ms:=50 -p compensate:=true \
  -p output:="$HOME/ros2_lab3_colcon/custom.csv"
```

В іншому терміналі:

```bash
source "$HOME/ros2_lab3_colcon/install/setup.bash"
ros2 control list_controllers
ros2 control set_controller_state position_p active
ros2 topic echo /joint_states --once
ros2 control set_controller_state position_p inactive
ros2 control set_controller_state position_p_predictor active
ros2 control list_controllers
ros2 control set_controller_state position_p_predictor inactive
```

## Docker

```bash
docker build -t ros2-lab3 .
docker run --rm -it ros2-lab3
```
