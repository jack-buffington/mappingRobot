#include <iostream>
#include <unistd.h>
#include <stdio.h>      // standard input / output functions
#include <string.h>     // string function definitions
#include <fcntl.h>      // File control definitions
#include <termios.h>    // POSIX terminal control definitions
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <vector>
#include <queue>
#include <thread>
#include <mutex>


#include <netdb.h>
#include <memory.h>  // needed for strlen  - May not be necessary depending on how I implement things
#include <errno.h>




// used just temporarily
#include <bitset>

class rangeData
	{
	public:
		unsigned int distanceInMillimeters;
		double angleInRadians;	
	};




class rplidarClass
	{
	public:
		rplidarClass();
		~rplidarClass();
		void startLidar();
		void stopLidar();
		void setPWM(int PWMvalue);
		bool hasGoodHealth();
		std::vector<rangeData> getMostRecentScan();
		unsigned char scanCount;


	private:
		int serialPort;
		std::vector<rangeData> mostRecentScan;
		bool shouldReadData; // tells the read thread if it should keep going
		std::mutex mostRecentScanMutex;
		

		const int DELAY_TIME_IN_MICROSECONDS = 4000;  // 2000 results in every 3rd or 4th read being successful
		const double DEGREES_TO_RADIANS_MULTIPLIER = 3.1415926535/180.0;

		int setupSerialPort();
		void sendCommand(char* buffer, int bufferSize, int serialPort);
		int readResponse(char* buffer, int sizeOfBuffer, int serialPort);
		void sendGetHealth(int serialPort);
		void sendRplidarPWM(int serialPort, unsigned int PWMvalue);
		void sendStop(int serialPort);
		void sendExpressScan(int serialPort);
		int calculateChecksum(char* buffer, int sizeOfBuffer);
		void readRangeData();
	}; // end of the rplidar class
