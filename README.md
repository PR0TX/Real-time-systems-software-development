# Лабораторна робота 1 · Вступ до ROS 2, налаштування середовища та розробка вузлів (Nodes)

## Огляд

| Параметр          | Значення                 |
| ----------------- | ------------------------ |
| Пакет             | `lab1_nodes`             |
| Дистрибутив ROS 2 | Lyrical                  |
| Мова              | C++17                    |
| Система збірки    | `ament_cmake` / `colcon` |

### Реалізовані вузли

- `lab1_nodes::Lab1Component` — компонент, який завантажується під час роботи;
- `/managed_node_a` — керований вузол A;
- `/managed_node_b` — керований вузол B.

## Швидкий запуск

```bash
cd 1/ros2_ws
source /opt/ros/lyrical/setup.bash
rosdep install --from-paths src --ignore-src -r -y

BUILD_ROOT=/home/ubuntu/ros2_lab1_colcon
rm -rf "$BUILD_ROOT"
colcon --log-base "$BUILD_ROOT/log" build --symlink-install \
  --build-base "$BUILD_ROOT/build" \
  --install-base "$BUILD_ROOT/install"

source "$BUILD_ROOT/install/setup.bash"
```

## Docker

```bash
cd 1
docker build -t ros2-lab1 .
docker run --rm ros2-lab1 \
  bash -c 'echo "ROS_DISTRO=$ROS_DISTRO"; ros2 pkg prefix lab1_nodes'
```

## Основні файли

```text
1/
├── Dockerfile
├── entrypoint.sh
├── CMakeLists.txt
├── package.xml
└── ros2_ws/src/lab1_nodes/
    └── src/
        ├── component_node.cpp
        ├── lifecycle_node_a.cpp
        └── lifecycle_node_b.cpp

```
