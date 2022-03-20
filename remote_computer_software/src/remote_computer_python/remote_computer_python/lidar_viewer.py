
# To build, do this from /remote_computer_software/src:
# colcon build 

# To run: 
# ros2 run remote_computer_python lidar_viewer 

# Test with:
# ros2 bag play <your bag's name>


import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from sensor_msgs.msg import LaserScan
import matplotlib.pyplot as plt
import matplotlib as plt2
import numpy as np
import math


class LidarViewer(Node):

    def __init__(self):
        super().__init__('lidar_viewer')

       # # To publish to this topic:
       # # ros2 topic pub /topic std_msgs/String "data: Test message..."
       # self.subscription = self.create_subscription(String, 'topic', self.listener_callback, 10)
       # self.subscription  # prevent unused variable warning


        # To test this, use a pre-recorded bag:
        # ros2 bag play walkingAround
        self.subscription = self.create_subscription(LaserScan, 'rplidarScan', self.lidarScanCallback, 10)
        self.subscription  # prevent unused variable warning

        fig = plt.figure(1)
        plt.show(block=False)


    def listener_callback(self, msg):
        self.get_logger().info('I heard: "%s"' % msg.data)

    def lidarScanCallback(self, msg):
        # Here is what is in the LaserScan message:
        # Header header            # timestamp in the header is the acquisition time of 
        #                          # the first ray in the scan.
        #                          #
        #                          # in frame frame_id, angles are measured around 
        #                          # the positive Z axis (counterclockwise, if Z is up)
        #                          # with zero angle being forward along the x axis
                                 
        # float32 angle_min        # start angle of the scan [rad]
        # float32 angle_max        # end angle of the scan [rad]
        # float32 angle_increment  # angular distance between measurements [rad]

        # float32 time_increment   # time between measurements [seconds] - if your scanner
        #                          # is moving, this will be used in interpolating position
        #                          # of 3d points
        # float32 scan_time        # time between scans [seconds]

        # float32 range_min        # minimum range value [m]
        # float32 range_max        # maximum range value [m]

        # float32[] ranges         # range data [m] (Note: values < range_min or > range_max should be discarded)
        # float32[] intensities    # intensity data [device-specific units].  If your
        #                          # device does not provide intensities, please leave
        #                          # the array empty.
        #self.get_logger().info('I heard: "%s"' % msg.data)
        self.get_logger().info('Received a lidar scan!')
        self.get_logger().info('min angle: "%f"' % msg.angle_min) 
        self.get_logger().info('min angle: "%f"' % msg.angle_max)
        self.get_logger().info('angle increment: "%f"' % msg.angle_increment)

        angleRange = msg.angle_max - msg.angle_min
        numberOfMeasurements = angleRange / msg.angle_increment
        numberOfMeasurementsInt = int(numberOfMeasurements)

        self.get_logger().info('# of measurements: "%d"' % numberOfMeasurements)

        # Create the X and Y coordinates from the scan data
        currentAngle = msg.angle_min # in radians


        cartesianCoords = np.zeros((numberOfMeasurementsInt, 2))
        


        for I in range(numberOfMeasurementsInt):
            distanceInMeters = msg.ranges[I]
            X = distanceInMeters * math.sin(currentAngle)
            Y = distanceInMeters * math.cos(currentAngle)
            currentAngle += msg.angle_increment
            cartesianCoords[I,:] = [X,Y]

        self.get_logger().info('Computed X/Y coordinates.')



        # Now plot those points!

        fig = plt.figure(1)
        plt.cla() # clear the previous drawing.
        plt.plot(cartesianCoords[:,0], cartesianCoords[:,1], 'o', color='black')
        plt.xlim(-6,6)
        plt.ylim(-6,6)
        fig.canvas.draw()
        fig.canvas.flush_events()

        #
        #plt.figure(1)
        
        #ax1 = plt.subplot(1,1,1)

        #plt.show(block=False)
        #plt.draw()



        self.get_logger().info('Plotted the points.')









def main(args=None):
    rclpy.init(args=args)

    lidar_viewer = LidarViewer()

    rclpy.spin(lidar_viewer)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    lidar_viewer.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

