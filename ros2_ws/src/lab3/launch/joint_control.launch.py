from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    config = Path(get_package_share_directory("lab3")) / "config"
    controller_params = str(config / "controllers.yaml")
    return LaunchDescription([
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            parameters=[{"robot_description": (config / "joint.urdf").read_text()}],
        ),
        Node(
            package="controller_manager",
            executable="ros2_control_node",
            parameters=[controller_params],
        ),
        Node(
            package="controller_manager",
            executable="spawner",
            arguments=["joint_state_broadcaster"],
        ),
        Node(
            package="controller_manager",
            executable="spawner",
            arguments=[
                "position_p", "position_p_predictor", "--inactive",
                "--param-file", controller_params,
            ],
        ),
    ])
