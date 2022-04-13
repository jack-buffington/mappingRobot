from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='mapping_robot',
            namespace='mapping_robot',
            executable='rplidarNode',
            name='rplidarNode'
        ),
        Node(
            package='mapping_robot',
            namespace='mapping_robot',
            executable='mcuNode',
            name='mcuNode'
        )
    ])
