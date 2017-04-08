sudo apt-get install emacs24
sudo apt-get install feh
sudo apt-get install raspi-gpio
sudo apt-get install screen
sudo apt-get install libopencv-dev python-opencv
git clone git://git.drogon.net/wiringPi
cd ~/wiringPi
./build 

####

raspistill 
raspivid -o /dev/null -t 0
raspistill -o pic2.png
ssh-keygen 
vi .ssh/authorized_keys
chmod 400 .ssh/authorized_keys 
wget -O /dev/null http://speedtest.wdc01.softlayer.com/downloads/test10.zip
raspivid -o video.h264 -fps 200 -w 320 -h 240 -t 10000 &
sudo modprobe bcm2835-v4l2
