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
const double coef[] = {0.000000014175208,  -0.000020224005030,   0.008447323489755,  -0.830402505433275,  34.128733821139527};

const int width = 640;
const int height = 178;
const int winSize = 72;
int maxPwm = 30;
int halfPeriod = 400;
int totalTime = 1000;

int iLowH = 6;
int iHighH = 20;

int iLowS =84; 
int iHighS = 255;

int iLowV = 20;
int iHighV = 255;

Mat imgOriginal;
VideoCapture cap(0); //capture the video from webcam
ofstream myfile;
stringstream ss;

void getInitPosition(int & x, int & y) {
  x = y = 0;
  while ( x < 10 || y < 10) {
    cap.read(imgOriginal);
    Mat imgHSV;
    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    Mat imgThresholded;
    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);  
      
    //morphological opening (removes small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

    //morphological closing (removes small holes from the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    Moments oMoments = moments(imgThresholded);

    double dM01 = oMoments.m01;
    double dM10 = oMoments.m10;
    double dArea = oMoments.m00;

    x = dM10 / dArea;
    y = dM01 / dArea;
  }
}

int initCam() {
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

int goRight() {
  pwmWrite(18, maxPwm);
  pwmWrite(13, 0);
}

int goLeft() {
  pwmWrite(18, 0);
  pwmWrite(13, maxPwm);
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

vector<Mat> mats;
int main( int argc, char** argv )
{
  setpriority(PRIO_PROCESS, 0, -21);
    
  initPwm();
  initCam();
  
  if (argc > 1) {
    maxPwm = atoi(argv[1]);
  }
  if (argc > 2) {
    halfPeriod = atoi(argv[2]);
  }
  if (argc > 3) {
    totalTime = atoi(argv[3]);
  }

  int x, y;  
  getInitPosition(x, y);
  

  long time = getTime();
  long clock = getTime();
  int dir = true;

  for (int i  = 0; i < totalTime; ++i) {
    if (getTime()-time > halfPeriod) {
      time = getTime();
      dir ? goRight() : goLeft();
      dir = !dir;
    }
    cap.read(imgOriginal);
    Mat img = imgOriginal(Rect(max(x - winSize/2, 0), max(y - winSize/2, 0), min(width - x, winSize), min(height - y, winSize)));
    // mats.push_back(img.clone());
    
    Mat imgHSV;
    cvtColor(img, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
 
    Mat imgThresholded;

    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);  
      
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
      x = max(x - winSize/2, 0) + newx;
      y = max(y - winSize/2, 0) + newy;
    } else {
      Mat imgHSV;
      cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
 
      Mat imgThresholded;
      inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);  
      
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
	x = newx;
	y = newy;
      }
    }

    long temp = getTime();
    double mult = 1;
    double l = 0;
    for (int i = 0; i < 5; ++i) {
      l += coef[4-i] * mult;
      mult *= x;
    }
    ss << temp -clock << "\t"<< x << "\t" << y << "\t" << l << "\t" << 1.07 * x - 394 << endl;
    clock = temp;
    // circle(imgLines, Point(x, y), 5, Scalar(0,0,255), 3);    
    // imgOriginal = imgOriginal + imgLines;
    // imwrite( "image" + to_string(i) + ".png", imgOriginal);
  }
  myfile.open ("out.txt");
  myfile << ss.rdbuf();
  myfile.close();
  stop();

  // for (auto i = mats.begin(); i != mats.end(); ++i) {
  //   imwrite( "image" + to_string(i-mats.begin()) + ".png", *i);
  // }
  return 0;
  
}
