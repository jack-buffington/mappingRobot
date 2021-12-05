/*
To get this to run, type the following from inside of the dev_ws directory:
. install/setup.bash
ros2 run mapping_robot rplidarNode


Subscribes to:
  rplidarOn -             Bool      1 is on, 0 is off
  rplidarPointsPerScan -  int32     The requested number of points per scan.  The actual value will be close to this value.
                                    It will use this value to determine a PWM value that sets the motor's spin rate.
  rplidarScanDivider -    int32     Publish every Nth scan. 1 publishes every scan

Publishes:
  rplidarScan -           LaserScan Various information about the scan in a standard format.  

*/

#include "rplidarNode.hpp"





  rplidarNode::rplidarNode() : Node("mcu_node")
  {
    // Callbacks for commands to this node
    rplidarOnSubscription = this->create_subscription<std_msgs::msg::Bool>("rplidarOn", MESSAGE_QUEUE_DEPTH, std::bind(&rplidarNode::rplidarOnCallback, this, _1));
    rplidarPointsPerScanSubscription = this->create_subscription<std_msgs::msg::Int32>("rplidarPointsPerScan", MESSAGE_QUEUE_DEPTH, std::bind(&rplidarNode::rplidarPointsPerScanCallback, this, _1));
    rplidarScanDividerSubscription = this->create_subscription<std_msgs::msg::Int32>("rplidarScanDivider", MESSAGE_QUEUE_DEPTH, std::bind(&rplidarNode::rplidarScanDividerCallback, this, _1));

    // Timer callback to see if there is a new scan ready.   
    scanCheckTimer = this->create_wall_timer(std::chrono::milliseconds(20), std::bind(&rplidarNode::scanCheckCallback, this));

    // Publisher for the scan data
    scanPublisher = this->create_publisher<sensor_msgs::msg::LaserScan>("rplidarScan", MESSAGE_QUEUE_DEPTH);

    scanDivider = 10; // Publish evey 10th scan by deafult.
    scanCount = 0;
    lastScanNumber = 100; // This needs to be any value other than 0 so that the first scan will be used.  
                           // In reality, this prevents an edge case that almost certainly wouldn't happen.
  }



  void rplidarNode::scanCheckCallback()
  { // This callback happens 50 times per second.   I am having it happen that often so that there is minimal delay 
    // between when the scan is complete and when it is published.   I really should rewrite this so that I don't have to poll 
    // for new results but that seems like more work than I want at this moment.  I previously wrote the rplidarClass for 
    // something else but it turned out that there wasn't a rplidar node for ROS2 foxy so I am just using what I previously 
    // wrote here, even if it isn't exactly perfect.   If you are seeing this note then I may have determined that the 
    // results were good enough for my application.  Either that or you are looking at this code shortly after I first posted 
    // it.

    // Check to see if there is a new scan
    if(lidar.scanCount != lastScanNumber)
    {
      lastScanNumber = lidar.scanCount;
      // Determine if this scan should be sent out
      if(++scanCount == scanDivider) // Don't confuse this scanCount with the one 3 lines ago...
      {
        // Send out this scan
        std::vector<rangeData> scan = lidar.getMostRecentScan();   
        /* Package the data into the correct format   
            Here is what is in the LaserScan message:
            Header header            # timestamp in the header is the acquisition time of 
                                     # the first ray in the scan.
                                     #
                                     # in frame frame_id, angles are measured around 
                                     # the positive Z axis (counterclockwise, if Z is up)
                                     # with zero angle being forward along the x axis
                                     
            float32 angle_min        # start angle of the scan [rad]
            float32 angle_max        # end angle of the scan [rad]
            float32 angle_increment  # angular distance between measurements [rad]

            float32 time_increment   # time between measurements [seconds] - if your scanner
                                     # is moving, this will be used in interpolating position
                                     # of 3d points
            float32 scan_time        # time between scans [seconds]

            float32 range_min        # minimum range value [m]
            float32 range_max        # maximum range value [m]

            float32[] ranges         # range data [m] (Note: values < range_min or > range_max should be discarded)
            float32[] intensities    # intensity data [device-specific units].  If your
                                     # device does not provide intensities, please leave
                                     # the array empty.
        */

        sensor_msgs::msg::LaserScan laserScanMessage;

        float startAngle = scan[0].angleInRadians;
        float endAngle = scan[scan.size() - 1].angleInRadians;
        int numberOfMeasurements = scan.size();

        laserScanMessage.angle_min = startAngle;
        laserScanMessage.angle_max = endAngle;
        laserScanMessage.angle_increment = (endAngle - startAngle) / numberOfMeasurements;

        // TODO: Figure out the correct time increment
        laserScanMessage.time_increment = 0;  // Time between one point and the next
        laserScanMessage.scan_time = 0;       // Time between one scan and the next
        laserScanMessage.range_min = 0.1;
        laserScanMessage.range_max = 16;      // I think that the reality is that it will give data for further than this 

        // Load up the ranges array.  
        // TODO:  This
        std::vector<float> ranges;
        for(int I = 0; I < numberOfMeasurements; ++I)
        {
          float tempF = scan[I].distanceInMillimeters;
          ranges.push_back(tempF / 1000);
        } 

        laserScanMessage.ranges = ranges;

        // Publish the message.          

        scanPublisher->publish(laserScanMessage);

        // Set things up for next time.
        scanCount = 0;
      }
    }

      

  }


  void rplidarNode::rplidarOnCallback(const std_msgs::msg::Bool::SharedPtr msg)
  { // to test:  ros2 topic pub -1 /displayMessage std_msgs/msg/String "data: 2~0~testing..."   TODO:   rewrite this line
    // A 1 turns on the rpLidar
    // A 0 turns off the rplidar
    RCLCPP_INFO(this->get_logger(), "rplidarOn: '%s'", msg->data);

    if(msg->data == false)
    {
      lidar.stopLidar();
    }
    else
    {
      lidar.startLidar();
      lidar.setPWM(600);  // You should wait about 4 seconds after doing this to give it time to spin up.  
    }

  }


  void rplidarNode::rplidarPointsPerScanCallback(const std_msgs::msg::Int32::SharedPtr msg)
  { // to test:  ros2 topic pub -1 /displayMessage std_msgs/msg/String "data: 2~0~testing..."   TODO:   rewrite this line
    // Pass this function how many points you would like to have per scan and it will choose a PWM value 
    // that will give you approximately that many points.   It can't be controlled exactly.
    // 100: 3.2 degrees per packet  3600 readings per revolution (roughly)
    // 200: 8.8                     1309
    // 300: 15.0                    768
    // 400: 21.2                    543
    // 500: 27.6                    417
    // 600: 33.9                    339
    // 700: 40.7                    283
    // 800: 48.3                    238
    // 900: 56.0                    205
    // 1000:63.7                    180 

    // The formula that I am using, which I found through messing around in a spreadsheet is
    // PWMvalue = ((1024 * 2792)/(samples - 60))^(1/1.45)

    int desiredPoints = msg->data;
    int PWMvalue;
    // Check to see if the number of samples is less than 180 or greater than 3600.  If so then just put out the limiting PWM values.  TODO:  This
    if(desiredPoints > 3600)
    {
      PWMvalue = 100;
    }
    else if (desiredPoints < 180)
    {
      PWMvalue = 1000;
    }
    else
    {
      float PWMvalueFloat = 2859008;
      PWMvalueFloat /= (desiredPoints - 60);
      PWMvalueFloat = pow(PWMvalueFloat, 1 / 1.45);

      PWMvalue = PWMvalueFloat;
    }
    lidar.setPWM(PWMvalue);
    
    RCLCPP_INFO(this->get_logger(), "rplidarPointsPerScan: '%s'", msg->data);

  }


  void rplidarNode::rplidarScanDividerCallback(const std_msgs::msg::Int32::SharedPtr msg)
  { // to test:  ros2 topic pub -1 /displayMessage std_msgs/msg/String "data: 2~0~testing..."   TODO:   rewrite this line
    RCLCPP_INFO(this->get_logger(), "rplidarScanDivider: '%s'", msg->data);
    scanDivider = msg->data;
  }



int main(int argc, char * argv[])
{
  std::cout << "version 001\n";

  // rplidarNode.lidar.startLidar();
  // rplidarNode.lidar.setPWM(600);
  // usleep(4*1000*1000); // 4 seconds to give it time to spin up


  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<rplidarNode>());
  rclcpp::shutdown();
  return 0;
}













