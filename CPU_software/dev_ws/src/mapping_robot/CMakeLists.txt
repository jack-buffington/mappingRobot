cmake_minimum_required(VERSION 3.5)
project(mapping_robot)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)




#add_executable(rplidarNode src/rplidarNode.cpp)
#target_include_directories(rplidarNode PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)


add_executable(rplidarNode  src/rplidarClass.cpp src/rplidarNode.cpp)
ament_target_dependencies(rplidarNode rclcpp std_msgs sensor_msgs)

add_executable(manualDrivingNode  src/commonlyUsedFunctions.cpp src/manualDrivingNode.cpp)
ament_target_dependencies(manualDrivingNode rclcpp std_msgs)

add_executable(mcuNode src/commonlyUsedFunctions.cpp src/serialPortStuff.cpp src/mcuNode.cpp)
ament_target_dependencies(mcuNode rclcpp std_msgs)



install(TARGETS
# include all nodes here.  Each one should be on a separate line.
  mcuNode
  rplidarNode
  manualDrivingNode
  DESTINATION lib/${PROJECT_NAME})

# Make it install the launch file.
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})




ament_package()
