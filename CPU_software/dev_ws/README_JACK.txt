If you have just cloned this repository then to create the other directories, just compile it by typing
colcon build

Only one node can be run from a terminal unless you run it in the background.  
To run a node, you first have to source setup.sh:
source install/setup.sh

Now you can run a node you have to do one of the following:
ros2 run mapping_robot mcuNode
ros2 run mapping_robot rplidarNode


If you wish to run all of the nodes at once then do the following from the dev_ws directory:
source install/setup.sh
ros2 launch mapping_robot mappingRobotLaunch.py

All of the code for the robot is located in mappingRobot/CPU_software/dev_ws/src/mapping_robot/src

Changes should be made to package.xml in mappingRobot/CPU_software/dev_ws/src/mapping_robot to give info about the package

Changes should also be made to CMakeLists.txt to specify the required packages and to specifically 
tell it to compile the two source files.


The launch file is located in 
/mappingRobot/CPU_software/dev_ws/install/mapping_robot/share/mapping_robot/launch/mappingRobotLaunch.py

To run that launch file:
From dev_ws:  source install/setup.bash
ros2 launch mapping_robot mapping_robot.launch.py

