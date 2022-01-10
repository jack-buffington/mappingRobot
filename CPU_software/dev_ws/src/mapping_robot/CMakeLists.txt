cmake_minimum_required(VERSION 3.5)
project(mapping_robot)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
#find_package(mapping_robot_interfaces REQUIRED)


include_directories(include)

#rosidl_generate_interfaces(${PROJECT_NAME}
#  "../mapping_robot_interfaces/msg/BeepType.msg"
#  DEPENDENCIES mapping_robot_interfaces
#)

# In this section, different nodes are created.  To add a new node, add a pair of lines like below.  
# Note how the add_executable line includes both the main source code as well as any other .cpp files that are needed.
# It also appears that the main node code must be the lastone listed or else you can't launch the node using ros2 run.

add_executable(mcuNode src/commonlyUsedFunctions.cpp src/serialPortStuff.cpp src/mcuNode.cpp)
ament_target_dependencies(mcuNode rclcpp std_msgs)

add_executable(rplidarNode  src/rplidarClass.cpp src/rplidarNode.cpp)
ament_target_dependencies(rplidarNode rclcpp std_msgs sensor_msgs)

add_executable(manualDrivingNode  src/commonlyUsedFunctions.cpp src/manualDrivingNode.cpp)
ament_target_dependencies(manualDrivingNode rclcpp std_msgs)


install(TARGETS
# include all nodes here.  Each one should be on a separate line.
  mcuNode
  rplidarNode
  manualDrivingNode
  DESTINATION lib/${PROJECT_NAME})

ament_package()
