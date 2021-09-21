This readme was last updated on August 8th 2019

This repository will contain all of the files necessary to make a mapping robot that can map its 
environment using a 2D lidar-like sensor.  At the time of writing this README, the project is 
ongoing.  Eventual goals of this project are:

* Have the robot autonomously map its environment using a RPlidar A2 module.  
* Know when it is done mapping its environment. 
* Do so using ROS.  
* Run exclusively my own code (except for ROS).
* Have the capability to connect to the robot using a remote computer which can view the current state of the
map as well as to take direct control of the robot.  

The robot system will consist of three processors.  The first is the remote computer which does no 
major computation.  It willbe for the user to view the map and to control the state of the robot if necessary.  
The second is an onboard processor that will do the heavy lifting and which holds 
the most up-to-date map.  The third is a microcontroller that handles low-level tasks such as driving the motors, 
reading sensors, and controllng the power to the motors, CPU, and itself. 

Power is supplied to the robot from an off-the-shelf battery pack for Dewalt cordless tools.  