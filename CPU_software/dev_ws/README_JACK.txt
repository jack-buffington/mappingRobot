All of the code for the robot is located in ~/dev_ws/src/mapping_robot/src

Changes should be made to package.xml in ~/dev_ws/src/mapping_robot to give info about the package

Changes should also be made to CMakeLists.txt to specify the required packages and to specifically 
tell it to compile the two source files.


The launch file is located in ~/mappingRobot/CPU_software/dev_ws/install/mapping_robot/share/mapping_robot

To run that launch file:
From dev_ws:  source install/setup.bash
ros2 launch mapping_robot mapping_robot.launch.py

