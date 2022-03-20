/*
Joystick driving node
Written by Jack Buffington February 2022

This file takes messages from a teleop_twist_joy node and converts them into driveMotors messages
so that the robot can be driven by joystick.  

To compile this node:  
  Go into remote_computer_software/src
  colcon build



  
To get this to run:
change directory to remote_computer_software/src
source install/setup.bash
ros2 run remote_computer joystick_driving_node

You also need to run the joystick node:
ros2 launch teleop_twist_joy teleop-launch.py joy_config:='xbox'



*/

#include "joystick_driving_node.hpp"




  joystickDrivingNode::joystickDrivingNode() : Node("joystick_driving_node")
  {
    // Subscription to the /joy messages
    joystickSubscription = this->create_subscription<sensor_msgs::msg::Joy>("joy", MESSAGE_QUEUE_DEPTH, std::bind(&joystickDrivingNode::joyCallback, this, _1));

    // Publisher so that I can send out motor messages
    motorPublisher = this->create_publisher<std_msgs::msg::Float32MultiArray>("driveMotors", MESSAGE_QUEUE_DEPTH);
  }



  void joystickDrivingNode::joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg)
  { // to test, just run the teleop_twist_joy node
    static int skipCount;
    float axis0 = msg->axes[2];
    float axis1 = msg->axes[1];   

    // Convert the joystick values into motor speeds.
    float turningScaleFactor = 1.0; 
    float leftMotor = axis1 - (axis0 * turningScaleFactor);
    float rightMotor = axis1 + (axis0 * turningScaleFactor);


    // The motor values need to be in meters per second so slow things down to a speed that the robot can deal with.
    float maximumSpeedAdjuster = 1.5;  // TODO:  Adjust this once I have the robot actually driving correctly in meters per second 
    leftMotor *= maximumSpeedAdjuster;
    rightMotor *= maximumSpeedAdjuster;



    
    // RCLCPP_INFO(this->get_logger(), "joystick axis 1: '%s'", std::to_string(axis1).c_str());


    // The joystick messages are coming in pretty quickly and although the MCU can handle the amount of messages just fine, I am paring down 
    // how often the motors get updated to free up the serial bus some more.

    if(skipCount % 3 == 0)
    {
      publishMotorMessage(leftMotor, rightMotor);
      //RCLCPP_INFO(this->get_logger(), "left motor: '%s'   rightMotor: '%s'", std::to_string(leftMotor).c_str(), std::to_string(rightMotor).c_str());
    }
    skipCount++;

  }

      



void joystickDrivingNode::publishMotorMessage(float leftMotor, float rightMotor)
{ // This publishes a message that is subscribed to by the MCU node which causes the motors to go. 
    std_msgs::msg::Float32MultiArray msg;
    msg.data.push_back(leftMotor);
    msg.data.push_back(rightMotor);
    motorPublisher->publish(msg);
}






int main(int argc, char * argv[])
{
  std::cout << "Joystick driving node Version 001 \n";

  //startTiming();

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<joystickDrivingNode>());
  rclcpp::shutdown();
  return 0;
}

