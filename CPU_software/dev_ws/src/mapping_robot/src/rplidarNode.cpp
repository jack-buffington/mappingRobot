/*
To get this to run, type the following from inside of the dev_ws directory:
. install/setup.bash
   


Subscribes to:
  rplidarOn -             Bool      1 is on, 0 is off
  rplidarPointsPerScan -  int32     The requested number of points per scan.  The actual value will be close to this value.
                                    It will use this value to determine a PWM value that sets the motor's spin rate.
  rplidarScanDivider -    int32     Publish every Nth scan. 1 publishes every scan

Publishes:
  rplidarScan -           LaserScan Various information about the scan in a standard format.  



  Testing:  

  The executable is in ~/mappingRobot/CPU_software/dev_ws/build/mapping_robot
  
  RUNNING IT:
  ros2 run mapping_robot rplidarNode

  TURNING IT ON OR OFF:
  ros2 topic pub -1 /rplidarOn std_msgs/msg/Bool "data: true"       
  ros2 topic pub -1 /rplidarOn std_msgs/msg/Bool "data: 0"

  CHANGING THE NUMBER OF POINTS PER SCAN:
  ros2 topic pub -1 /rplidarPointsPerScan std_msgs/msg/Int32 "data: 300"

  CHANGING HOW MANY SCANS ARE SKIPPED:
  ros2 topic pub -1 /rplidarScanDivider std_msgs/msg/Int32 "data: 1"

  RECORDING IT
  ros2 bag record -o testRecording /rplidarScan

  TODO:  It may be publishing 


*/

#include "rplidarNode.hpp"





  rplidarNode::rplidarNode() : Node("rplidar_node")  // What I have in Node is what shows up when I do ros2 node list if I ran this node by itself.
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
      if(++scanCount >= scanDivider) // Don't confuse this scanCount with the one 3 lines ago...
                                     // The > is because if the user requested a lower scan count, 
                                     // it might stop sending scans and count upward forever.
      {
        // Send out this scan
        std::vector<rangeData> scan = lidar.getMostRecentScan();  

        // Because ROS2's LaserScan message expects the readings to be at equal angles but the rplidar isn't necessarily
        // ouputting at equal angles, I'm binning the results here so that things are more correct.   I tried not doing this at first
        // but that result was HNR (Horrible And Wrong) :)   Basically the plotted points were swimming around because apparently the
        // velocity of the spinner changes a lot as it turns even though it doesn't seem like it is.   Either that or the rplidar is 
        // taking its readings at a non-uniform rate, which seems more likely now that I thought about it a bit more...  


        std::vector<int> adjustedScan(requestedNumberOfPoints,0);
        
        for(auto p:scan)
        { // round the angle and stick that measurement in the correct location


          float angle = p.angleInRadians;
          int distance = p.distanceInMillimeters;
          //std::cout << "angle: " << angle<< "distance: " << distance << std::endl;


          angle /= (2 * 3.14159);
          angle *= requestedNumberOfPoints;

          int angleInt = round(angle);

          

          if(angleInt >= requestedNumberOfPoints)
          {
            angleInt -= requestedNumberOfPoints;
          }
          else if(angleInt < 0)
          {
            angleInt += requestedNumberOfPoints;
          }



          if(adjustedScan[angleInt] != 0)
          {         
            if(adjustedScan[angleInt] > distance) // prioritize the shorter distance because that is what the robot will crash into first.
            {
              adjustedScan[angleInt] = distance;  
            }                            
          } 
          else
          { // Just stick the measurement in there.
            adjustedScan[angleInt] = distance; 
          }
        }

        

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


        float startAngle = 0;
        float endAngle = ((2 * 3.14159) / requestedNumberOfPoints) * (requestedNumberOfPoints - 1);  
        //unsigned int numberOfMeasurements = requestedNumberOfPoints;
        //std::cout << "Publishing a scan of " << std::dec << numberOfMeasurements << " points.\n";
        laserScanMessage.angle_min = startAngle;
        laserScanMessage.angle_max = endAngle;
        laserScanMessage.angle_increment = (2 * 3.14159) / requestedNumberOfPoints;
        // TODO: Figure out the correct time increment
        laserScanMessage.time_increment = 0;  // Time between one point and the next
        laserScanMessage.scan_time = 0;       // Time between one scan and the next
        laserScanMessage.range_min = 0.1;
        laserScanMessage.range_max = 16;      // I think that the reality is that it will give data for further than this 
        // Load up the ranges array.  
        std::vector<float> ranges;
        for(int I = 0; I < requestedNumberOfPoints; ++I)
        {
          float tempF = adjustedScan[I];
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
  { // to test:  ros2 topic pub -1 /rplidarOn std_msgs/msg/Bool "data: true" 
    // A 1 turns on the rpLidar
    // A 0 turns off the rplidar

    std::cout << "rplidarOn callback: " << msg->data << std::endl;

    if(msg->data == false)
    {
      lidar.stopLidar();
    }
    else
    {
      lidar.startLidar();
      lidar.setPWM(475);  // Gives about 375 points.  
      requestedNumberOfPoints = 360;
    }
  }


  void rplidarNode::rplidarPointsPerScanCallback(const std_msgs::msg::Int32::SharedPtr msg)
  { // to test:  ros2 topic pub -1 /rplidarPointsPerScan std_msgs/msg/Int32 "data: 300"
    // Pass this function how many points you would like to have per scan and it will choose a PWM value 
    // that will give you approximately 10% more so that the final output can have data in every bin.  

    // The best approximation that I could figure out in 10 minutes or so was:
    // number of points = (230000/PWM) - 85

    // or for what I am actually trying to solve for:
    // PWM = 230000/(number of points + 85)



    // Here are a bunch of scan results showing typical jitter 
    // Publishing a scan of 306 points.
    // Publishing a scan of 297 points.
    // Publishing a scan of 302 points.
    // Publishing a scan of 294 points.
    // Publishing a scan of 297 points.
    // Publishing a scan of 299 points.
    // Publishing a scan of 304 points.

    // Past about 1700, I have seen sporadic dropouts where I started to see random readings that were around 
    // 500 readings.   These appear to be partial scans for some reason.  That is how they look when I plot them out.

    std::cout << "rplidarPointsPerScanCallback - requested points: " << msg->data << std::endl;

    int desiredPoints = msg->data;
    requestedNumberOfPoints = desiredPoints;
    desiredPoints *= 1.1f; // Boost the number of points by 10% so that we can hit the target every time.  

    int PWMvalue;
    
    if (desiredPoints < 155)
    {
      PWMvalue = 1000;
    }
    else
    {
      float PWMvalueFloat = 230000;
      PWMvalueFloat /= (desiredPoints + 85);

      PWMvalue = PWMvalueFloat;
    }
    lidar.setPWM(PWMvalue);
    //lidar.setPWM(msg->data);

  }


  void rplidarNode::rplidarScanDividerCallback(const std_msgs::msg::Int32::SharedPtr msg)
  { // to test:  ros2 topic pub -1 /displayMessage std_msgs/msg/String "data: 2~0~testing..."   TODO:   rewrite this line
    //RCLCPP_INFO(this->get_logger(), "rplidarScanDivider: '%s'", msg->data);
    std::cout << "rplidarScanDividerCallback - value given: " << msg->data << std::endl; 
    scanDivider = msg->data;
  }



int main(int argc, char * argv[])
{
  std::cout << "rplidarNode - version 001\n";

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<rplidarNode>());
  rclcpp::shutdown();
  return 0;
}













