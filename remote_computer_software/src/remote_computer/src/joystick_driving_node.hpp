#ifndef JOYSTICK_DRIVING_NODE
#define JOYSTICK_DRIVING_NODE

	#include <memory>
	#include <unistd.h>
	#include <stdlib.h> // atoi, atof

	#include "rclcpp/rclcpp.hpp"


	#include "std_msgs/msg/float32_multi_array.hpp"
	#include "sensor_msgs/msg/joy.hpp"
	#include <vector>
	#include <iostream>



	#define KEY_DELAY 				250
	#define MESSAGE_QUEUE_DEPTH		10


	using std::placeholders::_1;


class joystickDrivingNode : public rclcpp::Node
{
public:
  joystickDrivingNode();

private:
  void publishMotorMessage(float leftMotor, float rightMotor);
  void joyCallback(const sensor_msgs::msg::Joy::SharedPtr msg) ;


  rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr motorPublisher; 
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joystickSubscription;

}; // End of the class


#endif
