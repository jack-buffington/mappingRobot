#ifndef RPLIDAR_NODE
#define RPLIDAR_NODE

	#include <memory>
	#include <unistd.h>
	#include <stdlib.h> // atoi, atof
	#include <math.h>

	#include "rclcpp/rclcpp.hpp"
	#include "serialPortStuff.hpp"
	#include "commonlyUsedFunctions.hpp"
	#include "std_msgs/msg/int32.hpp"
	#include "std_msgs/msg/bool.hpp"
	#include "sensor_msgs/msg/laser_scan.hpp"
	#include "rplidarClass.hpp"
	

	#define MESSAGE_QUEUE_DEPTH	10

	using std::placeholders::_1;


class rplidarNode : public rclcpp::Node
{
public:
  rplidarNode();


private:
  void rplidarOnCallback(const std_msgs::msg::Bool::SharedPtr msg);
  void rplidarPointsPerScanCallback(const std_msgs::msg::Int32::SharedPtr msg);
  void rplidarScanDividerCallback(const std_msgs::msg::Int32::SharedPtr msg);
  void scanCheckCallback();

  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr rplidarOnSubscription;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr rplidarPointsPerScanSubscription;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr rplidarScanDividerSubscription;


  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scanPublisher;

  int scanDivider;
  int PWM;  // TEMPORARY VARIABLE TO GET IT TO COMPILE
  rplidarClass lidar;
  
  rclcpp::TimerBase::SharedPtr scanCheckTimer;

}; // End of the class


#endif
