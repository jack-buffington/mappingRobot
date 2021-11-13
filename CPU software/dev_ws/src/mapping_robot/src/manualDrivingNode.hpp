#ifndef MANUAL_DRIVING_NODE
#define MANUAL_DRIVING_NODE

	#include <memory>
	#include <unistd.h>
	#include <stdlib.h> // atoi, atof

	#include "rclcpp/rclcpp.hpp"
	#include "std_msgs/msg/string.hpp"
	#include "serialPortStuff.hpp"
	#include "commonlyUsedFunctions.hpp"



	#define KEY_DELAY 				250
	#define MESSAGE_QUEUE_DEPTH		10


	using std::placeholders::_1;


class manualDrivingNode : public rclcpp::Node
{
public:
  manualDrivingNode();

private:
  void keyboardCallback();
  void publishMotorMessage(float leftMotor, float rightMotor);

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr motorPublisher;
  rclcpp::TimerBase::SharedPtr timer_;

}; // End of the class






#endif
