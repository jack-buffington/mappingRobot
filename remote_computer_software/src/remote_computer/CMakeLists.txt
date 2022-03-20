cmake_minimum_required(VERSION 3.5)
project(remote_computer)

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



include_directories(include)


add_executable(joystick_driving_node src/joystick_driving_node.cpp)
ament_target_dependencies(joystick_driving_node rclcpp std_msgs sensor_msgs)


#add_executable(joystick_driving_node src/joystick_driving_node.cpp)
#target_include_directories(joystick_driving_node PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>)

install(TARGETS 
  joystick_driving_node
  DESTINATION lib/${PROJECT_NAME})


ament_package()
