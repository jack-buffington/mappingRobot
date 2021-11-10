#ifndef MCU_NODE
#define MCU_NODE

	#include <memory>
	#include <unistd.h>
	#include <stdlib.h> // atoi, atof

	#include "rclcpp/rclcpp.hpp"
	#include "std_msgs/msg/string.hpp"
	#include "serialPortStuff.hpp"
	#include "commonlyUsedFunctions.hpp"


	#define TICKS_PER_METER    2000   // This is just a guess at this point.


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
  void driveMotorsCallback(const std_msgs::msg::String::SharedPtr msg) const;
  void cpuReadyCallback(const std_msgs::msg::String::SharedPtr msg) const;
  void serialTimerCallback();


  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr displayMessageSubscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr beepSubscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr driveMotorsSubscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr cpuReadySubscription;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr voltagePublisher;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr buttonPublisher;
}; // End of the class






#endif
