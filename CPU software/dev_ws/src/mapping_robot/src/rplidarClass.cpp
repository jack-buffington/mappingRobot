// This is an object that will work with a RPlidar A2

// Note that this code is currently not using the change in angle specified in the cabins
// because they don't make sense.  It is soley using the start angles to arrive at final angles.



#include "rplidarClass.hpp"
 

// ##########################
// #### PUBLIC FUNCTIONS ####
// ##########################

rplidarClass::rplidarClass()
	{
	// Locates a rplidar on serial ports 0 through 3 and connects to it.
	// stores the serial port integer in the private variable 'serialPort'.

	shouldReadData = false;
	scanCount = 0;
		
	char portName[] = {"/dev/ttyUSBX"};  // X will be replaced later
	serialPort = -1;



	//serialPort = std::open( portName, O_RDWR| O_NOCTTY );
	// Try the first ten USB serial ports
	for (int I = 0; I < 10; I++)
		{	
		char portNumber = '0' + I;
		portName[11] = portNumber;

		std::cout << "Trying to open " << portName << "... ";
		serialPort = open( portName, O_RDWR | O_NONBLOCK | O_NOCTTY ); // non-blocking
		if (serialPort == -1)
			{
			std::cout << "Failed." << std::endl;	
			}
		else
			{
			std::cout << "Succeeded!" << std::endl;


			struct termios tty;
			memset (&tty, 0, sizeof tty);

			/* Error Handling */
			if ( tcgetattr ( serialPort, &tty ) != 0 ) 
				{
				std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
				}

			/* Set Baud Rate */
			cfsetospeed (&tty, (speed_t)B115200); // output speed
			cfsetispeed (&tty, (speed_t)B115200); // input speed

			/* Setting other Port Stuff */
			tty.c_cflag     &=  ~PARENB;            // no parity
			tty.c_cflag     &=  ~CSTOPB;			// one stop bit
			tty.c_cflag     &=  ~CSIZE;				// prep for setting how many bits
			tty.c_cflag     |=  CS8;				// 8 bit

			tty.c_cflag     &=  ~CRTSCTS;           // no flow control
			

			tty.c_cc[VMIN]   =  1;                  // read doesn't block
			tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
			tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines


			/* Make raw */
			cfmakeraw(&tty);

			// The command above is equivalent to the following code:
			// termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
			//             | INLCR | IGNCR | ICRNL | IXON);
			// termios_p->c_oflag &= ~OPOST;
			// termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			// termios_p->c_cflag &= ~(CSIZE | PARENB);
			// termios_p->c_cflag |= CS8;



			/* Flush Port, then applies attributes */
			tcflush( serialPort, TCIFLUSH );
			if (tcsetattr ( serialPort, TCSANOW, &tty ) != 0) 
				{
			   	std::cout << "Error " << errno << " from tcsetattr" << std::endl;
				}



			// Check to see if a rplidar is on this port. 
			// Do it by sending get_health
			std::cout << "Checking to see if this port has a rplidar connected to it." << std::endl;
			sendGetHealth(serialPort);
			usleep(10000);  // delay 10 ms for a response to be sent.
			char buffer[40];
			readResponse(buffer, sizeof(buffer), serialPort); // read is non-blocking so this may or may not read anything
			
			int buffer1 = buffer[0];
			int buffer2 = buffer[1];
			int buffer3 = buffer[2];
			buffer1 &= 255;
			buffer2 &= 255;
			buffer3 &= 255;

			if (buffer1 == 0xA5 && buffer2 == 0x5A && buffer3 == 0x03)
				{
				std::cout << "Found a rplidar module." << std::endl;
				break;	
				}
			else
				{
				std::cout << "Didn't find a rplidar module on this port."; 
				close(serialPort);   	
				}
			
			}
		}

	// raise the dtr pin back up so that the motor can start.  
	if (serialPort != -1)
		{
		int DTR_flag = TIOCM_DTR;
   		ioctl(serialPort,TIOCMBIC,&DTR_flag); // Clear DTR pin 	
		}
	}



rplidarClass::~rplidarClass()
	{// stops the lidar and closes the serial port
	if (serialPort != -1)
		{
		stopLidar();
		close(serialPort);
		}
	
	}



void rplidarClass::startLidar()
	{
	// starts the rplidar's motor and makes it so that the vector 'mostCurrentScan' starts to be updated.
	sendRplidarPWM(serialPort,660); // A5 F0 02 94 02 C1
	// No response is sent for this command

	usleep(1000);

	sendExpressScan(serialPort); // A5 82 05 00 00 00 00 00 22
	char buffer[40];
	usleep(10000);  // give it time to reply
	readResponse(buffer, sizeof(buffer), serialPort); // Should be A5 5A 54 00 00 40 82

	shouldReadData = true;


	std::thread readThread(&rplidarClass::readRangeData, this);
	readThread.detach(); // make it so that this method can finish without crashing the program.
    //readThread.join(); // makes this thread wait until readThread finishes.   Not what I want.

	}




void rplidarClass::readRangeData()
	{// periodically reads the serial port and deals with the data that it receives
	 // This method runs in its own thread.

	char receivedBytes[200]; 	// It should never fill this buffer but I'm making it about 3x as big as it 
							// should be in case something else uses a lot of the processor and it sleeps
							// for longer than 10ms.
	int bytesRead;

	//char packetBuffer[84];
	//int bytesInPacketBuffer = 0;

	//rangeData tempData;

	unsigned int temp1, temp2;
	//int checksum;

	double lastAngle = 0;

	double lastCabinAngle = 600; // randomly large number that will force a new scan on receiving the first data


	std::vector<rangeData> partialScan;
	rangeData temp; // The first entry has a number saved in the distance variable so that the main loop
					// can know if the data is new or not.  
	temp.distanceInMillimeters = ++scanCount;
	partialScan.push_back(temp);


	while(shouldReadData)
		{// this loop keeps running until the program either terminates or the stop method is called

		// The packets start approximately every 8 milliseconds.  Realistically, if data was received every
		// 8 ms that would be OK.  This gives a maximum of about 16ms of lag between when a scan finishes 
		// and when it is available.   

		// Read data.  Fill the buffer until it has 84 bytes.  Evaluate for each byte after that. Clear the 
		// buffer once the data checks out.  
		std::this_thread::sleep_for(std::chrono::microseconds(DELAY_TIME_IN_MICROSECONDS)); // sleeps at least this amount of time
		bytesRead = readResponse(receivedBytes, sizeof(receivedBytes), serialPort); // read up to 200 bytes from the serial port.  (non-blocking)
		//std::cout << "Read " << std::dec << bytesRead << " bytes." << std::endl;


		// The number of bytes read will typically be -1 or 84.
		if (bytesRead == 84)
			{	
			//startByte = receivedBytes[0] & 0xF0 + (receivedBytes[1] & 0xF0 >> 4);
			temp1 = receivedBytes[0] & 0xF0;
			temp2 = receivedBytes[1] & 0xF0;


			//std::cout << "Start byte: " << std::hex << temp1 << " " << temp2 << std::endl;
			if (temp1 == 0xA0 && temp2 == 0x50)
				{// then the buffer could hold a valid packet.  Check the checksum.
				//std::cout << "Found a valid start byte." << std::endl;
				temp1 = receivedBytes[0] & 0x0F;
				temp2 = receivedBytes[1] & 0x0F;
				temp2 <<= 4;
				temp2 += temp1;
				unsigned int actualChecksum = calculateChecksum(&receivedBytes[2],82);
				//std::cout << "calculated Checksum: " << std::hex << temp1 << " " << temp2 << "   actual checksum: " << actualChecksum << std::endl;
				//char checksum = receivedBytes[0] & 0x0F + ((receivedBytes[1] & 0x0F) << 4);
				if (actualChecksum == temp2)
					{
					//std::cout << "Found a valid checksum." << std::endl;
					// Find the start angle for this packet
					// int startAngle1 = receivedBytes[2] & 0xFF;
					// int startAngle2 = receivedBytes[3] & 0x7F;
					// double startAngleDouble = startAngle2 * 256 + startAngle1;

					double startAngleDouble = ((receivedBytes[3] & 0x7F) * 256) + (receivedBytes[2] & 0xFF);
					startAngleDouble /= 64;
					//std::cout << std::dec << startAngle2 << "\t" << startAngle1 << "\t" << startAngleDouble << std::endl;
					//std::cout << startAngleDouble << "\t" << startAngleDouble - lastAngle << std::endl;

					double angleIncrement;

					if(startAngleDouble > lastAngle)
						angleIncrement = (startAngleDouble - lastAngle)/32;
					else
						angleIncrement = (startAngleDouble - lastAngle + 360)/32; 

					int cabinStart = 4;

					double currentBaseAngle = startAngleDouble;
					double actualAngle;
					double partialAngleInDegreesDouble = 0;
					rangeData dataPoint;


					for (int I = 0; I < 16; I++)
						{	
						// first reading in cabin
						int distanceInMillimeters = ((receivedBytes[cabinStart] & 0xFE) >> 1) + ((receivedBytes[cabinStart + 1] & 0xFF) * 128);
						// int partialAngleInDegrees = (receivedBytes[cabinStart + 4] & 0x0F);
						// if (receivedBytes[cabinStart] & 0x01 == 1)
						// 	partialAngleInDegrees = -partialAngleInDegrees;
						// double partialAngleInDegreesDouble = partialAngleInDegrees;
						// partialAngleInDegreesDouble /= 8;
						
						actualAngle = currentBaseAngle - partialAngleInDegreesDouble;
						if (actualAngle < 0)
							actualAngle += 360;


						// If this is the start of a new scan, copy the data over to mostRecentScan
						// TODO:   use mutex for this operation.
						if(actualAngle < lastCabinAngle)
							{// then this is the start of a new scan
								{
								std::lock_guard<std::mutex> lock(mostRecentScanMutex); 
								mostRecentScan = partialScan;
								} // these brackets are to make sure that mostRecentScan's usage is locked for as little time as possible 
							partialScan.clear();
							rangeData temp;
							temp.distanceInMillimeters = ++scanCount;
							partialScan.push_back(temp);
							}


						lastCabinAngle = actualAngle;

						actualAngle *= DEGREES_TO_RADIANS_MULTIPLIER;
						if(abs(distanceInMillimeters) > 1)
							{	
							dataPoint.distanceInMillimeters = distanceInMillimeters;
							dataPoint.angleInRadians = actualAngle;
							partialScan.push_back(dataPoint);
							}

						//std::cout << "Base: " << currentBaseAngle << "\tangle adjustment: " << partialAngleInDegreesDouble << "\tactual angle: " << actualAngle << std::endl;
						currentBaseAngle += angleIncrement;
						if (currentBaseAngle > 360)
							currentBaseAngle -= 360;





						// second reading in cabin
						distanceInMillimeters = ((receivedBytes[cabinStart+2] & 0xFE) >> 1) + ((receivedBytes[cabinStart + 3] & 0xFF) * 128);
						// partialAngleInDegrees = (receivedBytes[cabinStart + 4] & 0x0F);
						// if (receivedBytes[cabinStart+2] & 0x01 == 1)
						// 	partialAngleInDegrees = -partialAngleInDegrees;
						// partialAngleInDegreesDouble = partialAngleInDegrees;
						// partialAngleInDegreesDouble /= 8;
						
						actualAngle = currentBaseAngle - partialAngleInDegreesDouble;
						if (actualAngle < 0)
							actualAngle += 360;

						actualAngle *= DEGREES_TO_RADIANS_MULTIPLIER;
						if(abs(distanceInMillimeters) > 1)
							{	
							dataPoint.distanceInMillimeters = distanceInMillimeters;
							dataPoint.angleInRadians = actualAngle;
							partialScan.push_back(dataPoint);
							}



						//std::cout << "Base: " << currentBaseAngle << "\tangle adjustment: " << partialAngleInDegreesDouble << "\tactual angle: " << actualAngle << std::endl;
						currentBaseAngle += angleIncrement;
						if (currentBaseAngle > 360)
							currentBaseAngle -= 360;

						cabinStart += 5;
						}

					lastAngle = startAngleDouble;
					}
				}
			}
		}// end of while(shouldReadData)
	} // end of readRangeData()



void rplidarClass::stopLidar()
	{// stops the rplidar's motor and stops updating the 'mostCurrentScan' vector
	std::cout << "Stopping" << std::endl;
	sendStop(serialPort);
	sendRplidarPWM(serialPort,0);
	shouldReadData = false;  // This should stop the readRangeData loop.
	}



void rplidarClass::setPWM(int PWMvalue)
	{// changes the speed of rotation.  Acceptable values are from 0 to 1023.

	// Sending really low values will still have it run but it appears that the low limit is 
	// where there are about three degrees between packets.  This is at about 70 or so.


	// 100: 3.2 degrees per packet 	3600 readings per revolution (roughly)
	// 200: 8.8 		 			1309
	// 300: 15.0 		 			768
	// 400: 21.2 		 			543
	// 500: 27.6 		 			417
	// 600: 33.9 		 			339
	// 700: 40.7 		 			283
	// 800: 48.3 		 			238
	// 900: 56.0 		 			205
	// 1000:63.7 		 			180
	sendRplidarPWM(serialPort,PWMvalue);
	}



bool rplidarClass::hasGoodHealth()
	{// Checks the health of the rplidar. Returns true if there are no warnings or errors. 
	char buffer[10];

	sendGetHealth(serialPort); // A5 52
	readResponse(buffer, sizeof(buffer), serialPort); // Should be A5 5A 03 00 00 00 06 00 00 00 if things are OK.  
	// The rplidar data sheet only mentions A5 5A 03 00 00 00 06 but I am getting the extra three zeros afterwards.
	// The first byte after 0x03 is the status byte.  It can assume three values:  
	// 		0: good
	//		1: warning
	// 		2: Error
	// The next two bytes are the error code.  These codes are not documented so I've chosen to return true if 0 is received
	// and false if there is a warning or error.
	unsigned char tempChar = buffer[3] && 255;
	if (tempChar == 0)
		return true;
	else
		return false; 

	}



std::vector<rangeData> rplidarClass::getMostRecentScan()
	{
	std::lock_guard<std::mutex> lock(mostRecentScanMutex); 
	return mostRecentScan;
	}




// ###########################
// #### PRIVATE FUNCTIONS ####
// ###########################

int rplidarClass::readResponse(char* buffer, int sizeOfBuffer, int serialPort)
	{// reads up to sizeOfBuffer bytes from SerialPort
	 // returns how many bytes 
	int bytesRead = read(serialPort,buffer,sizeOfBuffer);

	//std::cout << "Response: ";
	// for(int I = 0; I < bytesRead; I++)
	// 	{
	// 	int val = buffer[I];
	// 	val &= 255;
		// std::cout << std::hex << val << " ";	
		// }
	//std::cout << std::endl;

	return bytesRead;

	}




void rplidarClass::sendGetHealth(int serialPort)
	{
	char buffer[2];
	buffer[0] = 0xA5;
	buffer[1] = 0x52;
	sendCommand(buffer, sizeof(buffer), serialPort);
	}




void rplidarClass::sendRplidarPWM(int serialPort, unsigned int PWMvalue)
	{// pwmValue is 0 to 1023
	char buffer[6];
	buffer[0] = 0xA5;
	buffer[1] = 0xF0;
	buffer[2] = 0x02;

	// figure out what values should go into the next three bytes.
	// The first byte is the low byte of PWMvalue.
	buffer[3] = PWMvalue;
	buffer[4] = PWMvalue >> 8;

	// Add a checksum
	buffer[5] = calculateChecksum(buffer, 5);

	sendCommand(buffer, sizeof(buffer), serialPort);
	}



// void sendGetAccessoryBoardFlag(int serialPort)
// 	{// I have no idea what this one does but it is based on the serial data that I read 
// 	 // with my logic analyzer and what I found in the documentation.
// 	char buffer[8];
// 	buffer[0] = 0xA5;
// 	buffer[1] = 0xFF;
// 	buffer[2] = 0x04;
// 	buffer[3] = 0x00;
// 	buffer[4] = 0x00;
// 	buffer[5] = 0x00;
// 	buffer[6] = 0x00;
// 	buffer[7] = 0x5E;
// 	sendCommand(buffer, sizeof(buffer), serialPort);
// 	}


// void sendGetInfo(int serialPort)
// 	{ // Gets information about the model, firmware, hardware, and serial number, 
// 	char buffer[2];
// 	buffer[0] = 0xA5;
// 	buffer[1] = 0x50;
// 	sendCommand(buffer, sizeof(buffer), serialPort);
// 	}


// void sendGetLidarConfiguration(int serialPort, int fourthByte)
// 	{// I have no idea what this one does but it is based on the serial data that I read 
// 	 // with my logic analyzer and what I found in the documentation.
// 	 // A5 84 24 (fourthByte 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 79
// 	char buffer[40];
// 	buffer[0] = 0xA5;
// 	buffer[1] = 0x84;
// 	buffer[2] = 0x24;
// 	buffer[3] = fourthByte; 
// 	buffer[4] = 0x00;
// 	buffer[5] = 0x00;
// 	buffer[6] = 0x00;
// 	buffer[7] = 0x00;
// 	buffer[8] = 0x00;
// 	buffer[9] = 0x00;
// 	buffer[10] = 0x00;
// 	buffer[11] = 0x00;
// 	buffer[12] = 0x00;
// 	buffer[13] = 0x00;
// 	buffer[14] = 0x00;
// 	buffer[15] = 0x00;
// 	buffer[16] = 0x00;
// 	buffer[17] = 0x00;
// 	buffer[18] = 0x00;
// 	buffer[19] = 0x00;
// 	buffer[20] = 0x00;
// 	buffer[21] = 0x00;
// 	buffer[22] = 0x00;
// 	buffer[23] = 0x00;
// 	buffer[24] = 0x00;
// 	buffer[25] = 0x00;
// 	buffer[26] = 0x00;
// 	buffer[27] = 0x00;
// 	buffer[28] = 0x00;
// 	buffer[29] = 0x00;
// 	buffer[30] = 0x00;
// 	buffer[31] = 0x00;
// 	buffer[32] = 0x00;
// 	buffer[33] = 0x00;
// 	buffer[34] = 0x00;
// 	buffer[35] = 0x00;
// 	buffer[36] = 0x00;
// 	buffer[37] = 0x00;
// 	buffer[38] = 0x00;
// 	buffer[39] = calculateChecksum(buffer,39);
// 	sendCommand(buffer, sizeof(buffer), serialPort);
// 	}




void rplidarClass::sendStop(int serialPort)
	{
	char buffer[2];
	buffer[0] = 0xA5;
	buffer[1] = 0x25;
	sendCommand(buffer, sizeof(buffer), serialPort);
	}



void rplidarClass::sendExpressScan(int serialPort)
	{
	char buffer[9];
	buffer[0] = 0xA5;
	buffer[1] = 0x82;
	buffer[2] = 0x05;
	buffer[3] = 0x00;
	buffer[4] = 0x00;
	buffer[5] = 0x00;
	buffer[6] = 0x00;
	buffer[7] = 0x00;
	buffer[8] = 0x22;
	sendCommand(buffer, sizeof(buffer), serialPort);
	}



int rplidarClass::calculateChecksum(char* buffer, int sizeOfBuffer)
	{// Calculates the checksum of a buffer.  If used to verify, will return 0 if it was correct.
	 // If used to create a checksum, will return the checksum value

	uint8_t checksum = 0;
	//uint8_t temp = 0;
	int tempInt;
	for (int I = 0; I < sizeOfBuffer; I++)
		{
		tempInt = buffer[I];
		checksum ^= tempInt;
		tempInt = checksum;

		}
	return tempInt;
	}


void rplidarClass::sendCommand(char* buffer, int bufferSize, int serialPort)
	{// sends all of the characters in the buffer to the serial port
	unsigned char theChar;


	std::cout << "Sent: ";

	for (int I = 0; I < bufferSize; I++)
		{
		theChar = buffer[I];
		int val = buffer[I] & 255;
		std::cout << std::hex << val << " ";
		int numberWritten = write(serialPort, &theChar,1);
		if (numberWritten != 1)
			std::cout << std::endl << "It didn't manage to write a byte to the serial port." << std::endl;
		}
	std::cout << std::endl;
	}


