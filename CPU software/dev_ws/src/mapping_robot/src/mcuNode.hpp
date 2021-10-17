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


#endif