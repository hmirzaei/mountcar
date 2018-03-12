#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <wiringPi.h>
#include <iostream>

using namespace cv;

void kalman(int m, double & l, double & ldot);
void processImage(VideoCapture & cap, int & x, int & y, bool windowed, bool fineProc);
void getInitPosition(VideoCapture & cap, int & x, int & y);

int initCam(VideoCapture & cap);
void initPwm();
void goRight(int maxPwm);
void goLeft(int maxPwm);
void stop();
