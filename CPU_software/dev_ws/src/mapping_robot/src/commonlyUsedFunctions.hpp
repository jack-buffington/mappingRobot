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

	//static struct termios old, current;
	//static struct timeval start, end;

	extern struct termios old, current;
	extern struct timeval start, end;

		
	union typeConverter
	{
		uint16_t	u16;
		int16_t		s16;
		uint8_t		bytes[2];
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
