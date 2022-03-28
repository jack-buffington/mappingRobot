ros2 pkg create --build-type ament_cmake <your package name> (optional: --dependencies <your dependencies> eg: --dependencies rclcpp std_msgs)


Adding the dependencies saves a bit of typing in CMakeLists.
Your package names should be all lower case, can include _ and numbers.  Failing to do this creates headaches.s
