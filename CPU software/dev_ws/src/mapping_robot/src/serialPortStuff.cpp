 /*	This file contains functions that relate to serial ports in a general sense. 
 

	0x55  <message length> <message type> <optional additional bytes> <checksum>
	Written by Jack Buffington 
	Last updated October 2021


*/


#include "serialPortStuff.hpp"

int serialPort;



bool setupSerialPort(char *thePort, bool lockIt)
{ // This sets up the serial port at 115200 baud 8N1
	// If lockIt is true then it makes it so that this program has exclusive access to the serial port.
	//std::cout << __func__ << " - Connecting to " << thePort << "\r\n";

	serialPort = open(thePort, O_RDWR);

	if(serialPort < 0)
	{
		std::string portString(thePort);
		std::cout << "Failed to connect to " << portString << std::endl;
		//std::cout << " - Failed!\n";
		std::string errorString(std::strerror(errno));
		std::cout << "Reason: " << errorString << std::endl;
		//perror ("Reason");
		//std::cout << "\r\n";
		return false;
	}


	// SETUP THE SERIAL PORT


	struct termios tty;
	memset(&tty, 0, sizeof tty);

	// Read in existing settings, and handle any error
	if(tcgetattr(serialPort, &tty) != 0) 
	{
		//std::cout << "Error from tcgetattr\n";
		return false;
	}


	tty.c_cflag |= CS8;		// 8-bits
	tty.c_cflag &= ~PARENB; // no parity
	tty.c_cflag &= ~CSTOPB; // one stop bit

	tty.c_cflag &= ~CRTSCTS; // no hardware flow control
	tty.c_cflag |= CREAD | CLOCAL; // Ignore modem-specific signals
	tty.c_lflag &= ~ICANON; // Make it process data immediately rather than when a NL is received
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	//tty.c_cc[VTIME] = 0;    // When reading, return immediately with what is in the buffer otherwise wait 
	//tty.c_cc[VMIN] = 1;	  // until something comes in.

	//tty.c_cc[VTIME] = 1;    // When reading, if something is in the buffer, return it immediately 
	//tty.c_cc[VMIN] = 0;	  // otherwise wait for up to 1/10th of a second for something to come in	

	tty.c_cc[VTIME] = 0;    // When reading, return immediately with what (if anything) is in the buffer 
	tty.c_cc[VMIN] = 0;



	cfsetispeed(&tty, B115200); // set the input and output baud rates
	cfsetospeed(&tty, B115200);


	// Save tty settings, also checking for error
	if (tcsetattr(serialPort, TCSANOW, &tty) != 0) 
	{
	    //std::cout << "Error from tcsetattr\n";
	    //std::cout << "Failed when trying to connect to " << thePort << "\r\n";
		std::string portString(thePort);
		std::cout << "Failed when trying to connect to " << portString << std::endl;
	    return false;
	}

	//std::cout << "Successfully connected to " << thePort << "\r\n";
	//std::cout << "File descriptor: " << serialPort << "\r\n";

	if(lockIt)
	{
		ioctl(serialPort, TIOCEXCL);
	}
	return true;
}



bool sendCharacterToSerialPort(char theChar)
{	// Sends a character to the serial port.  Returns true if successful.
	if(write(serialPort, &theChar, 1) == -1)
	{
		return false;
	}
	return true;
}



bool sendBufferToSerialPort(uint8_t* msg, int bufferSize)
{// Sends a buffer to the serial port.  Returns true if successful.

	std::cout << __func__ << std::endl;
	
	// For debugging purposes
	std::cout << "Sending this message: ";
	for(int I = 0; I < bufferSize; ++I)
	{
		std::cout << +msg[I] << ", ";
	}



	if(write(serialPort, msg, bufferSize) == -1)
	{
		return false;
	}
	return true;
}



bool readCharFromSerialPort(char *theChar, float timeoutInSeconds)
{ 	// Attempts to read a character from the serial port.  Returns true if this happens before the timeout.
	//std::cout << __func__ << " - starting\n";


	const clock_t startTime = clock();

	float elapsedTime;
	int bytesAvailable;

	while(1)
	{
		ioctl(serialPort, FIONREAD, &bytesAvailable);

		if(bytesAvailable)
		{
			read(serialPort, theChar, 1);
			//elapsedTime = float(clock () - startTime) /  CLOCKS_PER_SEC;
			//std::cout << "Took " << elapsedTime << " seconds to read the byte.\n";
			//std::cout << __func__ << " - done\n";
			return true;
		}

		elapsedTime = float(clock () - startTime) /  CLOCKS_PER_SEC;
		if(elapsedTime > timeoutInSeconds) // Give it time to respond then move on.  
		{
			//std::cout << __func__ << " - timeout\n";
			return false;
		}
	} // Done waiting for a byte to show up.
}




bool getMessageFromSerialPort(MCUmessage *theMessage)
{	// receives a message from a MCU on the serial port.  
	// The message format is 0x55 <# of bytes in the message past this one> <variable # of message bytes> <checksum>
	// Returns true if it found a message within a certain timeout duration.



	char theChar; 
	//std::cout << __func__ << " - start\n";

	const float initialWaitTimeInSeconds = 0.1; // was 0.05;

	bool lookingForStartByte = true;
	while(lookingForStartByte)
	{
		if(!readCharFromSerialPort(&theChar, initialWaitTimeInSeconds)) // If it times out while looking for a byte  
		{
			return false;
		}

		if(theChar == 0x55)
		{
			lookingForStartByte = false;
			std::cout << "Found a start byte!\n";
		}
	}



	// Read the message length
	if(!readCharFromSerialPort(&theChar, 0.005)) 
	{
		return false;
	}

	uint8_t packetLength = theChar;
	
	//std::cout << "Length of the packet: " << +packetLength << std::endl;

	theMessage->bytesInMessage = theChar;


	// Receive the rest of the packet.
	std::vector<uint8_t> messageBuffer;




	for(size_t I = 0; I < packetLength; ++I)
	{
		//std::cout << "Reading a character: ";
		if(!readCharFromSerialPort(&theChar, 0.005))
		{
			return false;
		}
		//std::cout << +theChar << std::endl;
		messageBuffer.push_back(theChar);
	}


	//std::cout << "Checking to see if the message is too big.\n";


	// return false if this message was too big for the buffer.
	if(messageBuffer.size() > MCU_BUFFER_SIZE)
	{
		return false;
	}


	char buffer[32];
	char *bufferPtr = buffer;

	//std::cout << "Copying into a buffer to check the checksum.\n";

	// Copy the temporary buffer into another that works for checking the checksum
	for(uint8_t I = 0; I < messageBuffer.size(); ++I)
	{
		*(bufferPtr + I) = messageBuffer[I];
	}


	// Check the checksum
	if(!checkChecksum((unsigned char*)buffer, packetLength))
	{
		std::cout << "Checksum failed.\n";
		return false;
	}

	//std::cout << "Checksum passed\n";

	for(uint8_t I = 0; I < messageBuffer.size() - 1; ++I) // -1 because I don't want the checksum
	{
		theMessage->message[I] = messageBuffer[I];
	}

	return true;
}



void computeChecksum(unsigned char *message, uint8_t messageSize)
{	// computes the checksum of the message starting with the byte in position 2.
	// For example, if the message is 0x55 0x02 0x06 0x00 then is starts with 0x06.
	// It then sticks the checksum into the last byte of the message.

	uint8_t checksum = 0;
	for(uint8_t  I = 2; I < messageSize - 1; ++I)
	{
		checksum ^= message[I];
	}

	message[messageSize - 1] = checksum;
}


bool checkChecksum(unsigned char *message, uint8_t messageSize)
{ 	// returns true if the checksum is good.
	// 
	uint8_t checksum = 0;
	for(size_t  I = 0; I < messageSize; ++I)
	{
		checksum ^= message[I];
		//std::cout << +message[I] << "  " << message[I] << "\n";
	}
	//std::cout << "Checksum:" << +checksum << "\n";
	//std::cout << "Message's checksum:" << +message[messageSize-1] << "\n";

	std::cout << "Computed checksum: " << +checksum << std::endl;

	return checksum == 0;
}


void flushSerialPort()
{
	tcflush(serialPort,TCIOFLUSH);
}


