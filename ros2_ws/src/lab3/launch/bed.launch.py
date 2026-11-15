from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def finish_mission(event, context):
    if context.is_shutdown:
        return None
    if event.returncode != 0:
        raise RuntimeError(f"Bed mission failed (exit code {event.returncode})")
    return [Shutdown(reason="Bed mission completed")]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("method", default_value="bt", choices=["bt", "fsm"]),
        DeclareLaunchArgument("season", default_value="winter", choices=["winter", "summer"]),
        DeclareLaunchArgument("has_pillows", default_value="true", choices=["true", "false"]),
        DeclareLaunchArgument("has_sheets", default_value="true", choices=["true", "false"]),
        DeclareLaunchArgument("clean_on_attempt", default_value="2", choices=["0", "1", "2", "3"]),
        Node(
            package="lab3",
            executable=["bed_", LaunchConfiguration("method")],
            parameters=[{"season": LaunchConfiguration("season")}],
            on_exit=finish_mission,
            output="screen",
        ),
        Node(
            package="lab3",
            executable="bed_service",
            parameters=[{
                "has_pillows": ParameterValue(LaunchConfiguration("has_pillows"), value_type=bool),
                "has_sheets": ParameterValue(LaunchConfiguration("has_sheets"), value_type=bool),
                "clean_on_attempt": ParameterValue(
                    LaunchConfiguration("clean_on_attempt"), value_type=int
                ),
            }],
            output="screen",
        ),
    ])
