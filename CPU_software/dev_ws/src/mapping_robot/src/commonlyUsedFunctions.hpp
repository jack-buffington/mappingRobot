#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

	#include <string>
	#include <vector>
	#include <algorithm>
	#include <stdio.h>
	#include <iostream>
	#include <fstream>
	#include <chrono>
	#include <ctime>
	#include <termios.h>
	#include <stdio.h>
	#include <sys/ioctl.h>  // defines some constants for reading keyboard
	#include <sys/time.h>
	#include "commonlyUsedFunctions.hpp"

	//static struct termios old, current;
	//static struct timeval start, end;

	extern struct termios old, current;
	extern struct timeval start, end;

		
	// typeConverter is used to convert to and from bytes for serial communication.   
	// Simply load up your bytes and then access the variable type that you want and vice versa.  

	union typeConverter
	{
		float	    f32;	// This isn't guaranteed to be 32-bit so verify on your system if you use this.
		uint32_t	u32;
		int32_t		s32;
		uint16_t	u16;
		int16_t		s16;
		uint8_t		bytes[3];
	};	
			

	std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
	std::string exec(const char* cmd, bool removeWhitespace, bool removeCR_LF);
	void printInGreen(std::string theText);
	void printInRed(std::string theText);
	void initTermios(int echo);
	void resetTermios(void);
	char getch_(int echo);
	char getch(void);
	char getche(void);
	int kbhit();
	void turnOffLocalEcho();
	void turnOnLocalEcho();
	void startTiming();
	int getElapsedTimeInMilliseconds();
	int getElapsedTimeInMicroseconds();
	void printElapsedTimeInMilliseconds();
	void printElapsedTimeInMicroseconds();
	
#endif
