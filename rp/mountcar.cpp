#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm>
#include <wiringPi.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream> 

using namespace cv;
using namespace std;

const int width = 640;
const int height = 178;
const int winSize = 72;

const int iLowH = 6;
const int iHighH = 20;

const int iLowS =84; 
const int iHighS = 255;

const int iLowV = 20;
const int iHighV = 255;

int kalman(int m, double & l, double & ldot) {
  //        0.5356 z^2 - 0.3397 z + 1.131e-16
  // x1_e:  ---------------------------------
  //             z^2 - 1.268 z + 0.4644
  
  //         17.04 z^2 - 17.04 z
  // x2_e:  ----------------------
  //        z^2 - 1.268 z + 0.4644
  static double l1, l2, ldot1, ldot2;
  static int m1, m2;
  l    = 0.5356 * m - 0.3397 * m1 + 1.131e-16 * m2 + 1.268 * l1    - 0.4644 * l2;
  ldot = 17.040 * m - 17.040 * m1                  + 1.268 * ldot1 - 0.4644 * ldot2;

  m2 = m1;
  m1 = m;
  l2 = l1;
  l1 = l;
  ldot2 = ldot1;
  ldot1 = ldot;
}

void processImage(VideoCapture & cap, int & x, int & y, bool windowed, bool fineProc) {
  Mat imgOriginal;
  Mat imgHSV;
  Mat imgThresholded;
  Mat img;

  if (!windowed) 
    x = y = 0;

  cap.read(imgOriginal);
  if (windowed)
    img = imgOriginal(Rect(max(x - winSize/2, 0), max(y - winSize/2, 0), min(width - x, winSize), min(height - y, winSize)));
  else
    img = imgOriginal;
    
  cvtColor(img, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
  inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);  

  if (fineProc) {
    //morphological opening (removes small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
    
    //morphological closing (removes small holes from the foreground)
    dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    Moments oMoments = moments(imgThresholded);
    double dM01 = oMoments.m01;
    double dM10 = oMoments.m10;
    double dArea = oMoments.m00;
    
    x = dM10 / dArea;
    y = dM01 / dArea;
    return;
  } else {
    while (true) {
      Mat nonZeroCoordinates;
      findNonZero(imgThresholded, nonZeroCoordinates);
      int newx = 0;
      int newy = 0;
      for (int i = 0; i < nonZeroCoordinates.total(); i++ ) {
	newx +=  nonZeroCoordinates.at<Point>(i).x;
	newy +=  nonZeroCoordinates.at<Point>(i).y;
      }
      if (nonZeroCoordinates.total() > 0) {
	newx /= nonZeroCoordinates.total();
	newy /= nonZeroCoordinates.total();
	if (windowed) {
	  x = max(x - winSize/2, 0) + newx;
	  y = max(y - winSize/2, 0) + newy;
	} else {
	  x = newx;
	  y = newx;
	}
	break;
      } else {
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);  
      }
    }
  }
}

void getInitPosition(VideoCapture & cap, int & x, int & y) {
  x = y = 0;
  while ( x < 10 || y < 10) {
    processImage(cap, x, y, false, true);
  }
}

int initCam(VideoCapture & cap) {
  cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  cap.set(CV_CAP_PROP_FPS, 200);
  if ( !cap.isOpened() )  // if not success, exit program
    {
      cout << "Cannot open the web cam" << endl;
      return -1;
    } 
}

int initPwm() {
  wiringPiSetupGpio();
  pinMode(18, PWM_OUTPUT);
  pinMode(13, PWM_OUTPUT);
  pwmSetClock(20);
  pwmSetRange(100);
  
  pwmWrite(18, 0);
  pwmWrite(13, 0);
}

int goRight(int maxPwm) {
  pwmWrite(13, maxPwm);
  pwmWrite(18, 0);
}

int goLeft(int maxPwm) {
  pwmWrite(13, 0);
  pwmWrite(18, maxPwm);
}

int stop() {
  pwmWrite(18, 0);
  pwmWrite(13, 0);
}

long getTime()
{
  struct timeval start;
  long mtime, seconds, useconds;

  gettimeofday(&start, NULL);
  seconds  = start.tv_sec;
  useconds = start.tv_usec;

  mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
  return mtime;
}

bool controlLaw(double l, double ldot) { // returns direction
  if (abs(ldot) < 50) {
    if (abs(l) < 6) {
      return false;
    } else {
      if (l > 0) {
	return false;
      } else {
	return true;
      }
    }
  } else {
    return (ldot > 0);
  }
}

int main( int argc, char** argv )
{
  int goal = -130;
  int totalTime = 10000;
  int maxPwm = 30;

  Mat imgOriginal;
  VideoCapture cap(0); //capture the video from webcam
  ofstream myfile;
  stringstream ss;
  
  setpriority(PRIO_PROCESS, 0, -20);
  initPwm();
  initCam(cap);
  
  if (argc > 1) {
    goal = atoi(argv[1]);
  }
  if (argc > 2) {
    totalTime = atoi(argv[2]);
  }
  if (argc > 3) {
    maxPwm = atoi(argv[3]);
  }

  int x, y;  
  getInitPosition(cap, x, y);
  long time = getTime();
  long clock = getTime();
  bool dir = true;
  int counter = 0;

  for (int i  = 0; i < totalTime; ++i) {
    processImage(cap, x, y, true, false);
    
    long temp = getTime();
    double mult = 1;
    double l, ldot;

    kalman(1.07 * x - 394, l, ldot);
    dir = controlLaw(l, ldot);
    dir ? goRight(maxPwm) : goLeft(maxPwm);

    ss << temp -clock << "\t"<< x  << "\t" << 1.07 * x - 394 << "\t" << l << "\t" << ldot << "\t" << (int)dir*100 << endl;
    clock = temp;

    if (l <= goal)  break;
  }
  myfile.open ("out.txt");
  myfile << ss.rdbuf();
  myfile.close();
  stop();

  return 0;
  
}
