This readme was last updated on March 20 2022.

This repository will eventually contain all of the files necessary to make a mapping robot that can map its 
environment using a 2D lidar-like sensor.  At the time of writing this README, the project is 
ongoing.  Eventual goals of this project are:

* Have the robot autonomously map its environment using a RPlidar A2 module.  
* Know when it is done mapping its environment. 
* Do so using ROS.  
* With only a few minor exceptions, run exclusively my own code.
* Have the capability to connect to the robot using a remote computer which can view the current state of the
map as well as to take direct control of the robot.  

The robot system will consist of three processors.  The first is the remote computer which does no 
major computation.  It will be for the user to view the map and to control the state of the robot if necessary.  
The second is an onboard processor that will do the heavy lifting and which holds 
the most up-to-date map.  The third is a microcontroller that handles low-level tasks such as driving the motors, 
reading sensors, and controllng the power to the motors, CPU, and itself. 

Power is supplied to the robot from an off-the-shelf battery pack for Dewalt cordless tools.  

It its current state, the MCU code is pretty much fully functional.  I will likely update it at some point to
read one or more downward-looking range finders so that it doesn't attempt suicide by driving down steps.  It currently
can do the following:
   * Read the battery voltage, display it on the OLED display, and send the voltage to the CPU
   * Read the encoders attached to the motors and send their values to the CPU
   * Drive the motors with a closed-loop velocity controller
   * Play beeps
   * Control the power to the CPU, motors, and itself
   * Communicate through two serial ports.  One to the CPU and one connected to the programming port.
  
The CPU code has:
   RPlidar node - controls a RPlidar A2 and reads its data.  This data is conditionally published based on a divider that 
       Allows it to send every N scans.  
   MCU node - Subscribes to a few topics which can be forwarded to the MCU and publishes a couple others which repeat 
       messages received from the MCU in a ROS-friendly format.  

The Remote computer code has:
   lidar viewer - Ideally I would use RVIZ to see the lidar data but either I don't yet know how to use RVIZ or I have it
	misconfigured so I wrote a quick and dirty program to display the scans.  
   joystick driving node - Lets you control the robot with a joystick if you have teleop_twist_joy installed on your system.
    
