#!/bin/bash
# This gets copied to /usr/bin

echo "Startup script - Sourcing the setup file."
source /usr/bin/mappingRobotSetup.sh
echo "Startup script - launching the launch file."
ros2 launch /home/ubuntu/mappingRobot/CPU_software/dev_ws/src/mapping_robot/launch/mappingRobotLaunch.py

