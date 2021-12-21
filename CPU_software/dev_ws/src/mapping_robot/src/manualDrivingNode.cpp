/*
To get this to run, type the following from inside of the dev_ws directory:
. install/setup.bash
  


"

*/

#include "manualDrivingNode.hpp"




  manualDrivingNode::manualDrivingNode() : Node("manual_driving_node")
  {
    // Callback for monitoring the keyboard
    timer_ = this->create_wall_timer(std::chrono::milliseconds(50), std::bind(&manualDrivingNode::keyboardCallback, this));

    // Publisher so that I can send out motor messages
    motorPublisher = this->create_publisher<std_msgs::msg::Float32MultiArray>("driveMotors", MESSAGE_QUEUE_DEPTH);
  }




  void manualDrivingNode::keyboardCallback()
  {
    // This callback happens 20 times per second. It checks for keyboard input  
    char c;
    static char lastC = 0;
    static bool stopSent = true;
    static float speed = 0.1;
    float leftMotor = 0;
    float rightMotor = 0;

    while(kbhit()) // If a key has been pressed
    {
      startTiming();
      c = getchar();
      if (lastC != c)
      {
        stopSent = false;
        switch (c)
        {
          case('a'):
            std::cout << "left turn\n";
            leftMotor = -speed;
            rightMotor = speed;
            std::cout << "Motors: " << leftMotor << ", " << rightMotor  << std::endl;
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case('w'):
            std::cout << "forward\n";
            leftMotor = speed;
            rightMotor = speed;
            std::cout << "Motors: " << leftMotor << ", " << rightMotor  << std::endl;
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case('d'):
            std::cout << "right turn\n";
            leftMotor = speed;
            rightMotor =- speed;
            std::cout << "Motors: " << leftMotor << ", " << rightMotor  << std::endl;
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case('s'):
            std::cout << "backward\n";
            leftMotor = -speed;
            rightMotor = -speed;
            std::cout << "Motors: " << leftMotor << ", " << rightMotor  << std::endl;
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case(' '):
            std::cout << "stop\n";
            leftMotor = 0;
            rightMotor = 0;
            std::cout << "Motors: " << leftMotor << ", " << rightMotor  << std::endl;
            publishMotorMessage(leftMotor, rightMotor);
            break;

          // The speeds are meant to change things between directional button pushes, not when one is being actively being pushed.
          case('1'):
            std::cout << "Speed 0.1\n";
            speed = 0.1;
            break;
          case('2'):
            std::cout << "Speed 0.25\n";
            speed = 0.25;
            break;
          case('3'):
            std::cout << "Speed 0.5\n";
            speed = 0.5;
            break;
          case('4'):
            std::cout << "Speed 1.0\n";
            speed = 1.0;
            break;
          case('5'):
            std::cout << "Speed 1.5\n";
            speed = 1.5;
            break;
          case('6'):
            std::cout << "Speed 2.0\n";
            speed = 2.0;
            break;
          case('7'):
            std::cout << "Speed 3.0\n";
            speed = 3.0;
            break;
          case('8'):
            std::cout << "Speed 4.0\n";
            speed = 4.0;
            break;



        }
        
      }
      lastC = c;
    } // end of while statement

    //std::cout << "time:" << getElapsedTimeInMilliseconds() << std::endl << std::flush;
   //usleep(10000); // 10 ms
    if(getElapsedTimeInMilliseconds() > KEY_DELAY && stopSent == false)
    {
      stopSent = true;
      std::cout << "STOP\n";
      publishMotorMessage(0,0);
      lastC = 0;
      leftMotor = 0;
      rightMotor = 0;
    }
    
  }


void manualDrivingNode::publishMotorMessage(float leftMotor, float rightMotor)
{
    std_msgs::msg::Float32MultiArray msg;
    msg.data.push_back(leftMotor);
    msg.data.push_back(rightMotor);
    motorPublisher->publish(msg);
}






int main(int argc, char * argv[])
{
  std::cout << "Version 002 - Uses the Float32MultiArray instead of a string.\n";

  turnOffLocalEcho();
  startTiming();

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<manualDrivingNode>());
  rclcpp::shutdown();
  return 0;
}

