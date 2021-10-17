#ifndef SERIAL_PORT
#define SERIAL_PORT


#define MCU_BUFFER_SIZE		32

	#include <iostream>
	#include <cerrno>
	#include <termios.h> 
	#include <ctime>
	#include <fcntl.h> // Contains file controls like O_RDWR
	#include <unistd.h> // write(), read(), close()
	#include <sys/ioctl.h> // FIONREAD
	#include <cstring> // memset 
	#include <vector>


	extern int 			serialPort;
	//static struct 	termios old, current;


	struct MCUmessage
	{
	   int bytesInMessage;
	   char message[MCU_BUFFER_SIZE]; // This will include all data between bytesInMessage and the checksum.
	};


	bool setupSerialPort(char *thePort, bool lockIt);
	bool readCharFromSerialPort(char *theChar, float timeoutInSeconds);
	bool getMessageFromSerialPort(MCUmessage *theMessage);
	void computeChecksum(unsigned char *message, uint8_t messageSize);
	bool checkChecksum(unsigned char *message, uint8_t messageSize);
	bool sendBufferToSerialPort(uint8_t* msg, int bufferSize);
	bool sendCharacterToSerialPort(char theChar);
	void flushSerialPort();
#endif

