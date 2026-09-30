# Лабораторна №2 - Комунікаційні інтерфейси та QoS

## Збірка

```bash
cd "/home/ubuntu/Desktop/LABS/Real-time systems software development/Laboratory/2"
bash build.sh
```

У терміналі:

```bash
source /opt/ros/lyrical/setup.bash
source /home/ubuntu/ros2_lab2_colcon/install/setup.bash
```

## Файли

```text
2/
├── README.md
├── build.sh
├── Dockerfile
├── entrypoint.sh
├── .dockerignore
├── .gitignore
├── .vscode/settings.json
└── ros2_ws/src/
    ├── lab2_interfaces/
    │   ├── CMakeLists.txt
    │   ├── package.xml
    │   ├── srv/ResetFilter.srv
    │   └── action/ProcessArray.action
    └── lab2_nodes/
        ├── CMakeLists.txt
        ├── package.xml
        ├── src/data_filter.cpp
        ├── src/reset_filter_client.cpp
        ├── src/array_action_server.cpp
        ├── src/pointcloud_components.cpp
        ├── include/lab2_nodes/pointcloud_type_adapter.hpp
        ├── config/qos_overrides.yaml
        ├── launch/pointcloud_inter_process.launch.py
        └── launch/pointcloud_intra_process.launch.py
```

## Docker

```bash
cd "/home/ubuntu/Desktop/LABS/Real-time systems software development/Laboratory/2"
docker build -t ros2-lab2 .
docker run --rm -it --network host ros2-lab2 ros2 run lab2_nodes data_filter

docker run --rm -it --network host ros2-lab2 \
  ros2 launch lab2_nodes pointcloud_intra_process.launch.py
```
