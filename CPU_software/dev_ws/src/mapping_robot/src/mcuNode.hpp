#ifndef MCU_NODE
#define MCU_NODE

	#include <memory>
	#include <unistd.h>
	#include <stdlib.h> // atoi, atof

	#include "rclcpp/rclcpp.hpp"
	#include "std_msgs/msg/string.hpp"
	#include "serialPortStuff.hpp"
	#include "commonlyUsedFunctions.hpp"
	#include "std_msgs/msg/float32_multi_array.hpp"
	//#include "mapping_robot_interfaces/msg/BeepType.hpp"


	#define TICKS_PER_METER    	18909   	// This is the average of four measured runs where the robot ran between 2 and 3 meters.   
	#define MESSAGE_QUEUE_DEPTH	10

	void beep(int whichBeep);
	void sendTextToDisplay(int row, int col, std::string theText);
	void driveMotors(float leftMotor, float rightMotor);

	using std::placeholders::_1;


class MCUnode : public rclcpp::Node
{
public:
  MCUnode();

private:
  void displayMessageCallback(const std_msgs::msg::String::SharedPtr msg) const;
  void beepCallback(const std_msgs::msg::String::SharedPtr msg) const;        
  void driveMotorsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) const;
  void cpuReadyCallback(const std_msgs::msg::String::SharedPtr msg) const;
  void serialTimerCallback();


  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr displayMessageSubscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr beepSubscription;
  rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr driveMotorsSubscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr cpuReadySubscription;

  rclcpp::TimerBase::SharedPtr timer_;
  
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr voltagePublisher;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr buttonPublisher;
	rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr encoderPublisher;


}; // End of the class






#endif
