from launch import LaunchDescription
from launch_ros.actions import Node
def generate_launch_description():
    ld = LaunchDescription()
    mcu_node = Node(package="mapping_robot", executable="mcuNode" )
    #rplidar_node = Node(package="mapping_robot", executable="rplidarNode")
    ld.add_action(mcu_node)
    #ld.add_action(rplidar_node)
    return ld

#  Reference: https://roboticsbackend.com/ros2-launch-file-example/