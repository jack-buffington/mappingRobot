echo "copying setup.sh to /usr/bin."
sudo cp dev_ws/install/setup.sh /usr/bin/mappingRobotSetup.sh

echo "Copying the startup script that is called by the service to /usr/bin."
sudo cp mappingRobotStartupScript.sh /usr/bin






echo "Stopping the serivce if it already exists."
sudo systemctl stop mappingRobot.service

echo "Copying the service to the proper location."
sudo cp mappingRobot.service /etc/systemd/system/

echo "Enabling the service."
sudo systemctl daemon-reload 
sudo systemctl enable mappingRobot.service

echo "Starting the service."
sudo systemctl start mappingRobot.service

echo "All done!  Enjoy playing with your robot!"
