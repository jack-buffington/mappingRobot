To make the minimal publisher/subscriber, I followed the instructions on this web page:  

https://docs.ros.org/en/foxy/Tutorials/Writing-A-Simple-Cpp-Publisher-And-Subscriber.html

To get things to run, it isn't clear on the web page but in each terminal you have to run 

. install/setup.bash

then either 

ros2 run cpp_pubsub talker

or

ros2 run cpp_pubsub listener



All of the code is located in ~/dev_ws/src/cpp_pubsub/src

Changes should be made to package.xml in ~/dev_ws/src/cpp_pubsub to give info about the package

Changes should also be made to CMakeLists.txt to specify the required packages and to specifically 
tell it to compile the two source files.

