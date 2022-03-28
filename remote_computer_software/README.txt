If you are trying to compile this and it isn't working because it is the first time that you have tried to compile, try this:

source ~/ros2_foxy/install/setup.sh
colcon build


For some reason sourcing the setup.sh in ./remote_computer_software/install isn't initially making ros2 available. 
Once you have done your first build, sourcing ./remote_computer_software/install/setup.sh will work. 
