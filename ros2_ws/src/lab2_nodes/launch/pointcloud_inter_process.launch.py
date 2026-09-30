from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    # Два контейнери працюють у різних процесах(перетворення у PointCloud2)
    return LaunchDescription([
        ComposableNodeContainer(
            name='pointcloud_publisher_container', namespace='',
            package='rclcpp_components', executable='component_container', output='screen',
            composable_node_descriptions=[ComposableNode(
                package='lab2_nodes', plugin='lab2_nodes::PointCloudPublisherComponent',
                extra_arguments=[{'use_intra_process_comms': False}])]),
        ComposableNodeContainer(
            name='pointcloud_subscriber_container', namespace='',
            package='rclcpp_components', executable='component_container', output='screen',
            composable_node_descriptions=[ComposableNode(
                package='lab2_nodes', plugin='lab2_nodes::PointCloudSubscriberComponent',
                extra_arguments=[{'use_intra_process_comms': False}])]),
    ])
