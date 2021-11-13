/*
To get this to run, type the following from inside of the dev_ws directory:
. install/setup.bash
ros2 run mapping_robot manualDrivingNode


"

*/

#include "manualDrivingNode.hpp"




  manualDrivingNode::manualDrivingNode() : Node("manual_driving_node")
  {
    // Callback for monitoring the keyboard
    timer_ = this->create_wall_timer(std::chrono::milliseconds(50), std::bind(&manualDrivingNode::keyboardCallback, this));

    // Publisher so that I can send out motor messages
    motorPublisher = this->create_publisher<std_msgs::msg::String>("driaveMotors", MESSAGE_QUEUE_DEPTH);
  }




  void manualDrivingNode::keyboardCallback()
  {
    // This callback happens 20 times per second. It checks for keyboard input  
    char c;
    static char lastC = 0;
    static bool stopSent = true;
    static int speed = 0.1;
    float leftMotor = 0;
    float rightMotor = 0;

    if(kbhit()) // If a key has been pressed
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
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case('w'):
            std::cout << "forward\n";
            leftMotor = speed;
            rightMotor = speed;
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case('d'):
            std::cout << "right turn\n";
            leftMotor = speed;
            rightMotor =- speed;
            publishMotorMessage(leftMotor, rightMotor);
            break;
          case('s'):
            std::cout << "backward\n";
            leftMotor = -speed;
            rightMotor = -speed;
            publishMotorMessage(leftMotor, rightMotor);
            break;

          // The speeds are meant to change things between directional button pushes, not when one is being actively being pushed.
          case('1'):
            std::cout << "Speed 1\n";
            speed = 0.1;
            break;
          case('2'):
            std::cout << "Speed 2\n";
            speed = 0.25;
            break;
          case('3'):
            std::cout << "Speed 3\n";
            speed = 0.5;
            break;
          case('4'):
            std::cout << "Speed 4\n";
            speed = 1.0;
            break;


        }
        
      }
      lastC = c;
    }
    //std::cout << "time:" << getElapsedTimeInMilliseconds() << std::endl << std::flush;
    usleep(10000); // 10 ms
    if(getElapsedTimeInMilliseconds() > KEY_DELAY && stopSent == false)
    {
      stopSent = true;
      std::cout << "STOP\n";
      lastC = 0;
      leftMotor = 0;
      rightMotor = 0;
    }
    
  }


void manualDrivingNode::publishMotorMessage(float leftMotor, float rightMotor)
{
    auto message = std_msgs::msg::String();
    message.data = std::to_string(leftMotor) + "," + std::to_string(rightMotor);
    motorPublisher->publish(message);
}






int main(int argc, char * argv[])
{
  std::cout << "version 001\n";

  turnOffLocalEcho();
  startTiming();

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<manualDrivingNode>());
  rclcpp::shutdown();
  return 0;
}

