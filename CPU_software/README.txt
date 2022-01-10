To compile, enter the following from this directory:

bash ./compileIt.sh

That script sets the date in case it is wrong so that colcon doesn't complain.  It also deletes previous build files 
so that you end up with something that is based on your most recent code.  It then does the compile.

To run your nodes, enter the following from the dev_ws directory (for example)
. install/setup.bash 
ros2 run mapping_robot mcuNode


To set things up so that they run at startup do this command once:
sudo bash makeRobotBeActiveOnBoot.sh
