
# I am setting the date because colcon build is complaining about dates being in the future if I don't do this.
echo "nameserver 8.8.8.8" | sudo tee /etc/resolv.conf
sudo date -s "$(wget -qSO- --max-redirect=0 google.com 2>&1 | grep Date: | cut -d' ' -f5-8)Z"

# I am removing these directoriw because I have found that it won't always build the newest code if I don't
rm -r dev_ws/build/mapping_robot
rm -r dev_ws/install/mapping_robot

cd dev_ws
colcon build
cd ..
