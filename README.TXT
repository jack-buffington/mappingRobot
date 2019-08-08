This readme was last updated on August 8th 2019

This repository will contain all of the files necessary to make a mapping robot that can map its 
environment using a 2D lidar-like sensor.  At the time of writing this README, the project is 
ongoing.  Eventual goals of this project are:

* Have the robot autonomously map its environment using a RPlidar A2 module.   
* Do so using ROS.  
* Run exclusively my own code.
* Be able to connect to the robot using a remote computer which can view the current stat of the
map as well as to take direct control of the robot.  

The robot system will consist of three processors.  The first is the remote computer which does no 
major computation.  The second is an onboard processor that will do the heavy lifting and which holds 
the most up-to-date map.  The third is a low-power processor (currently an arduino) that drives the 
motors, monitors the battery state, and turns on/off the power to the motors, the main processor, and
itself.  

Power is supplied to the robot from an off-the-shelf battery pack for a cordless drill.  
