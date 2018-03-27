sudo apt-get install emacs24
# sudo apt-get install feh
# sudo apt-get install raspi-gpio
# sudo apt-get install screen
sudo apt-get install libopencv-dev python-opencv
# sudo apt-get install libav-tools
git clone git://git.drogon.net/wiringPi
git config --global user.name Hamid
git config --global user.email hmirzai@gmail.com
cd wiringPi
./build 

####

echo alias emacs=\"emacs -nw\" >> ~/.bashrc
echo sudo modprobe bcm2835-v4l2 >> ~/.bashrc


raspistill 
raspivid -o /dev/null -t 0
raspistill -o pic2.png
ssh-keygen 
vi .ssh/authorized_keys
chmod 400 .ssh/authorized_keys 
wget -O /dev/null http://speedtest.wdc01.softlayer.com/downloads/test10.zip
raspivid -o video.h264 -fps 200 -w 320 -h 240 -t 10000 &
avconv -y -r 10 -i image%d.png -r 10 -vcodec mpeg4 -q:v 10 video.mp4
time sudo ./mountcar
/opt/vc/bin/raspivid -rot 180 -vf -t 0 -hf -fps 30 -w 640 -h 480 -o - | gst-launch-1.0 -e fdsrc ! h264parse ! rtph264pay pt=96 config-interval=5 ! udpsink host=169.234.205.190 port=5000
