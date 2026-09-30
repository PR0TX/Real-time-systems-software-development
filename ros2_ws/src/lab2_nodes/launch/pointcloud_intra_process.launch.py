from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    # обидва в одному процесі
    return LaunchDescription([ComposableNodeContainer(
        name='pointcloud_container', namespace='',
        package='rclcpp_components', executable='component_container', output='screen',
        composable_node_descriptions=[
            ComposableNode(package='lab2_nodes',
                           plugin='lab2_nodes::PointCloudPublisherComponent',
                           extra_arguments=[{'use_intra_process_comms': True}]),
            ComposableNode(package='lab2_nodes',
                           plugin='lab2_nodes::PointCloudSubscriberComponent',
                           extra_arguments=[{'use_intra_process_comms': True}]),
        ],
    )])
