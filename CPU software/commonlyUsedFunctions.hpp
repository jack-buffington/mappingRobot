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

	
	union typeConverter
{
	uint16_t	u16;
	int16_t		s16;
	uint8_t		bytes[2];
};	
	
	//extern std::ofstream logFile;
	//extern std::string logFileName;

	//void initLogFile();
	//void logAndPrint(std::string theString);
	std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
	std::string exec(const char* cmd, bool removeWhitespace, bool removeCR_LF);
	void printInGreen(std::string theText);
	void printInRed(std::string theText);
	//std::string getTimeStringForLogging();
	
#endif
