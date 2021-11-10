/*
To get this to run, type the following from inside of the dev_ws directory:
. install/setup.bash
ros2 run mapping_robot mcuNode


To publish on a topic from the command line:
ros2 topic pub -1 /beep std_msgs/msg/String "data: this is a test"

*/

#include "mcuNode.hpp"




  MCUnode::MCUnode() : Node("mcu_node")
  {
    // Callbacks for things that go to the MCU
    displayMessageSubscription = this->create_subscription<std_msgs::msg::String>("displayMessage", MESSAGE_QUEUE_DEPTH, std::bind(&MCUnode::displayMessageCallback, this, _1));
    driveMotorsSubscription = this->create_subscription<std_msgs::msg::String>("driveMotors", MESSAGE_QUEUE_DEPTH, std::bind(&MCUnode::driveMotorsCallback, this, _1));
    beepSubscription = this->create_subscription<std_msgs::msg::String>("beep", MESSAGE_QUEUE_DEPTH, std::bind(&MCUnode::beepCallback, this, _1));
    cpuReadySubscription = this->create_subscription<std_msgs::msg::String>("cpuReady", MESSAGE_QUEUE_DEPTH, std::bind(&MCUnode::cpuReadyCallback, this, _1));
    
    // Callback for monitoring received serial data
    timer_ = this->create_wall_timer(std::chrono::milliseconds(50), std::bind(&MCUnode::serialTimerCallback, this));
    
    // Publishers that are used when a message is received from the MCU
    voltagePublisher = this->create_publisher<std_msgs::msg::String>("batteryVoltage", MESSAGE_QUEUE_DEPTH);
    buttonPublisher = this->create_publisher<std_msgs::msg::String>("buttonPress", MESSAGE_QUEUE_DEPTH);
  }


  void MCUnode::displayMessageCallback(const std_msgs::msg::String::SharedPtr msg) const
  { // to test:  ros2 topic pub -1 /displayMessage std_msgs/msg/String "data: 2~0~testing..."
    RCLCPP_INFO(this->get_logger(), "displayMessage: '%s'", msg->data.c_str());

    std::vector<std::string> splits = splitString(msg->data, "~");  // I chose a tilde because it is unlikely to be in a message
    uint8_t row = atoi(splits[0].c_str());
    uint8_t col = atoi(splits[1].c_str());
    std::string theMessage = splits[2];
    sendTextToDisplay(row,col, theMessage);
  }



  void MCUnode::beepCallback(const std_msgs::msg::String::SharedPtr msg) const        
  { // To test:  ros2 topic pub -1 /beep std_msgs/msg/String "data: 0"
    RCLCPP_INFO(this->get_logger(), "beep: '%s'", msg->data.c_str());
    int whichBeep = atoi(msg->data.c_str());
    beep(whichBeep);
  }



  void MCUnode::driveMotorsCallback(const std_msgs::msg::String::SharedPtr msg) const
  { // to test:  ros2 topic pub -1 /driveMotors std_msgs/msg/String "data: 1.0,1.0"
    // The values are in meters per second.
    RCLCPP_INFO(this->get_logger(), "driveMotors: '%s'", msg->data.c_str());

    std::vector<std::string> splits = splitString(msg->data, ",");
    float leftMotor = atof(splits[0].c_str());
    float rightMotor = atof(splits[1].c_str());
    driveMotors(leftMotor, rightMotor);
  }


  void MCUnode::cpuReadyCallback(const std_msgs::msg::String::SharedPtr msg) const
  {
    RCLCPP_INFO(this->get_logger(), "cpuReady: '%s'", msg->data.c_str());
  }




  void MCUnode::serialTimerCallback()
  {
    // This callback happens 20 times per second.   
    // This implements a state machine that is used to keep track of where it was in a serial message.

    //RCLCPP_INFO(this->get_logger(), "Serial timer callback...");

    static char messageBuffer[32]; // 32 is more than enough I think
    static uint8_t bytesInMessage;
    static uint8_t bytesReceived;
    static uint8_t state = 0;  
    static uint8_t checksum; 

    char theChar;

    while(readCharFromSerialPort(&theChar, 0.001)) // time out in 1 ms.
    {
      // Show the bytes received for debug purposes
      //std::cout << +theChar << ", " << std::flush;
      switch(state)
      {
        case 0: // Start byte
          if(theChar == 85)
          {
            state = 1;
            checksum = 0;
            bytesReceived = 0;
          }
          break;
        case 1: // bytes in message
          bytesInMessage = theChar;
          state = 2;
          break;
        case 2:
          messageBuffer[bytesReceived] = theChar;
          checksum ^= theChar;
          ++bytesReceived;
          if(bytesReceived == bytesInMessage)
          {
            if(checksum == 0)
            { // Then it has passed the checksum
              state = 0;
              uint8_t messageType = messageBuffer[0];
              switch (messageType)
              {
                case 1: // Button presses
                  {
                  // '0x55 <bytes in message> 0x01 <number of button presses> <checksum>
                  std::cout << "Received a button press message.  " << +messageBuffer[1] << " presses.\n";
                  // Publish this on a topic. 
                  auto message = std_msgs::msg::String();
                  message.data = "Number of button presses: " + std::to_string(messageBuffer[1]);
                  std::cout << "Publishing the button press.\n";
                  buttonPublisher->publish(message);
                  break;
                  }

                case 2: // Battery voltage - This is received periodically whether I want it or not. :)
                  {
                  // Battery voltage is four bytes and represents voltage * 1000
                  uint32_t voltageValue = messageBuffer[1] + (messageBuffer[2] << 8) + (messageBuffer[3] << 16) + (messageBuffer[4] << 24);
                  float voltageFloat = voltageValue;
                  voltageFloat /= 1000;
                  std::cout << "Received a voltage message: " << voltageFloat << "V\n"; 
                  // Publish this on a topic. 
                  auto message = std_msgs::msg::String();
                  message.data = "Battery voltage: " + std::to_string(voltageFloat);
                  std::cout << "Publishing the voltage\n";
                  voltagePublisher->publish(message);
                  break;
                  }
              }
            }
          }
      } // End of the switch statement - No bracket is missing above.  
    }
  }









int main(int argc, char * argv[])
{
  std::cout << "version 018\n";
  std::string portString = "/dev/serial1";
  if(setupSerialPort((char*)portString.c_str(), true))
  {
    std::cout << "Successfully opened the serial port!\n";
  }


  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MCUnode>());
  rclcpp::shutdown();
  return 0;
}









void beep(int whichBeep)
{  // Causes the MCU to beep in the requested pattern
   // 0xAB <bytes in message> 0x01 <which beep pattern> <checksum>
   uint8_t messageBuffer[5];
   messageBuffer[0] = 171; // 0xAB
   messageBuffer[1] = 3;
   messageBuffer[2] = 1;
   messageBuffer[3] = whichBeep;
   computeChecksum(messageBuffer, sizeof(messageBuffer));
   sendBufferToSerialPort(messageBuffer, sizeof(messageBuffer));  
}




void sendTextToDisplay(int row, int col, std::string theText)
{  // 0xAB <bytes in message> 0x00 <row> <column> <message> <checksum>
  std::cout << __func__ << std::endl;

   // Do some prep first
   int bytesInMessage = theText.length();
   bytesInMessage += 4; // +4 for message type, row, col, and checksum

   uint8_t messageBuffer[32]; // bigger than it needs to be;

   messageBuffer[0] = 171; //0xAB
   messageBuffer[1] = bytesInMessage;
   messageBuffer[2] = 0;   // display message
   messageBuffer[3] = row;
   messageBuffer[4] = col;
   int msgPos = 5;
   for(unsigned int I = 0; I < theText.length(); ++I)
   {
      messageBuffer[msgPos++] = theText.at(I);
   }

   computeChecksum(messageBuffer, bytesInMessage + 2);
   sendBufferToSerialPort(messageBuffer, bytesInMessage + 2);  
}



void driveMotors(float leftMotor, float rightMotor)
{  // Drives the motors in meters per second
   // 0xAB <bytes in message> 0x03 <left velocity in ticks per second as long> <right velocity in ticks per second as long><checksum>

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
   messageBuffer[0] = 171; // 0xAB
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
