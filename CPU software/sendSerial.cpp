/*  sendSerial.cpp

Compile with 

g++ sendSerial.cpp serialPortStuff.cpp -o sendSerial

*/


#include <stdio.h>
#include <string.h>
#include "serialPortStuff.hpp"


// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close(), usleep
#include <iostream> // substr

#include <chrono>
#include <sys/time.h>
#include <ctime>

// For beeps
#define HAPPY  0
#define SAD    1

// This is just a guess at this point.
#define TICKS_PER_METER    2000




void sendTextToDisplay(int row, int col, std::string theText);
void beep(int whichBeep);
void driveMotors(float leftMotor, float rightMotor);
void clearDisplay();
float requestBatteryVoltage();
MCUmessage receiveMCUmessage();


int main()
{

   std::string portString = "/dev/serial1";
   if(setupSerialPort((char*)portString.c_str(), true))
   {
      std::cout << "Successfully opened the serial port!\n";

      sendTextToDisplay(0,0," Driving motors ");
      sendTextToDisplay(1,0,"    Forward     ");
      sendTextToDisplay(2,0,"                ");
      sendTextToDisplay(3,0,"                ");

      beep(HAPPY);
      sleep(1);
      beep(SAD);
      sleep(1);

      driveMotors(1.0,1.0);
      sendTextToDisplay(1,0,"    Forward    ");
      sleep(2);
      driveMotors(-1.0,-1.0);
      sendTextToDisplay(1,0,"    Backward    ");
      sleep(2);
      driveMotors(0,0);
      sendTextToDisplay(1,0,"    Stopped     ");


      float batteryVoltage = requestBatteryVoltage();
      std::cout <<  "Voltage: " << batteryVoltage << std::endl;

      std::string voltageString;

      voltageString = std::to_string(batteryVoltage);
      std::cout << voltageString << std::endl;


      std::string trimmedVoltage = voltageString.substr(0,5);
      trimmedVoltage += "           ";

      sendTextToDisplay(0,0," Battery Voltage");
      sendTextToDisplay(1,0, trimmedVoltage);
      sendTextToDisplay(2,0,"                ");
      sendTextToDisplay(3,0,"                ");

      close(serialPort);
   }
   else
   {
      std::cout << "Failed to open the serial port.\n";
   }
}



void clearDisplay()
{ // erases everything on the display
   sendTextToDisplay(0,0,"                ");
   sendTextToDisplay(1,0,"                ");
   sendTextToDisplay(2,0,"                ");
   sendTextToDisplay(3,0,"                ");
   usleep(20000); // Just to give the serial port some time to catch up on the Propeller
}



void sendTextToDisplay(int row, int col, std::string theText)
{  // 0x55 <bytes in message> 0x00 <row> <column> <message> <checksum>

   // Do some prep first
   int bytesInMessage = theText.length();
   bytesInMessage += 4; // +4 for message type, row, col, and checksum


   uint8_t messageBuffer[bytesInMessage + 2]; // +2 because of header, and bytesInMessage 
   messageBuffer[0] = 85; //0x55
   messageBuffer[1] = bytesInMessage;
   messageBuffer[2] = 0;   // display message
   messageBuffer[3] = row;
   messageBuffer[4] = col;
   int msgPos = 5;
   for(int I = 0; I < theText.length(); ++I)
   {
      messageBuffer[msgPos++] = theText.at(I);
   }
   computeChecksum(messageBuffer, sizeof(messageBuffer));
   sendBufferToSerialPort(messageBuffer, sizeof(messageBuffer));  
}



void beep(int whichBeep)
{  // Causes the MCU to beep in the requested pattern
   // 0x55 <bytes in message> 0x01 <which beep pattern> <checksum>
   uint8_t messageBuffer[5];
   messageBuffer[0] = 85; // 0x55
   messageBuffer[1] = 3;
   messageBuffer[2] = 1;
   messageBuffer[3] = whichBeep;
   computeChecksum(messageBuffer, sizeof(messageBuffer));
   sendBufferToSerialPort(messageBuffer, sizeof(messageBuffer));  
}



void driveMotors(float leftMotor, float rightMotor)
{  // Drives the motors in meters per second
   // 0x55 <bytes in message> 0x03 <left velocity in ticks per second as long> <right velocity in ticks per second as long><checksum>

   // Convert from meters per second to ticks per second
   int leftMotorInt = leftMotor * TICKS_PER_METER;
   int rightMotorInt = rightMotor * TICKS_PER_METER;

   union intSplitter
   {
      int32_t  theInt;
      char  theChars[4];
   };

   intSplitter splitter;

   uint8_t messageBuffer[12];
   messageBuffer[0] = 85; // 0x55
   messageBuffer[1] = 10;  // bytes in message
   messageBuffer[2] = 3;   // drive motors

   splitter.theInt = leftMotorInt;
   messageBuffer[3] = splitter.theChars[0];
   messageBuffer[4] = splitter.theChars[1];
   messageBuffer[5] = splitter.theChars[2];
   messageBuffer[6] = splitter.theChars[3];

   splitter.theInt = rightMotorInt;
   messageBuffer[7] = splitter.theChars[0];
   messageBuffer[8] = splitter.theChars[1];
   messageBuffer[9] = splitter.theChars[2];
   messageBuffer[10] = splitter.theChars[3];

   computeChecksum(messageBuffer, sizeof(messageBuffer));
   sendBufferToSerialPort(messageBuffer, sizeof(messageBuffer));  

}



float requestBatteryVoltage()
{  // Requests the battery's voltage from the MCU
   // 0x55 0x02 0x06 0x06
   std::cout << "Requesting the battery's voltage\n";
   uint8_t messageBuffer[4];
   messageBuffer[0] = 85; // 0x55
   messageBuffer[1] = 2;  // bytes in message
   messageBuffer[2] = 6;   // request voltage

   computeChecksum(messageBuffer, sizeof(messageBuffer));
   sendBufferToSerialPort(messageBuffer, sizeof(messageBuffer));  


   //std::cout << "Looking for a voltage request response.\n"; 

   // Search for the response
   MCUmessage theResponse = receiveMCUmessage();

  // std::cout << "Received a response, parsing it.\n";

   // for(int I = 0; I < 4; ++I)
   // {
   //    std::cout << "byte " << I << ": " << +theResponse.message[I] << std::endl;
   // }

   // The value returned is the voltage * 1000
   // Convert to a float.
   // I'm assuming that the 0th byte will say that it is a voltage response
   unsigned int  c4 = theResponse.message[4];
   unsigned int  c3 = theResponse.message[3];
   unsigned int  c2 = theResponse.message[2];
   unsigned int  c1 = theResponse.message[1];



   int voltageInt = c1 + (c2 * 256) + (c3 * 65536) + (c4 * 16777216);

   //std::cout << "voltageInt: " << voltageInt << std::endl;

   float voltage = voltageInt;

   return voltage / 1000;
}

MCUmessage receiveMCUmessage()
{  // Receives a message from the MCU, checks its checksum, and then returns the message.
   // It will wait for up to timeoutInMilliseconds after each byte received before failing.

   MCUmessage theMessage;

   std::cout << "About to call getMessageFromSerialPort.\n";
   while(!getMessageFromSerialPort(&theMessage));
   {
      std::cout << "    calling...\n";
   }
   
   std::cout << "    Done.\n";

   return theMessage;
}