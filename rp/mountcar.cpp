#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm>

using namespace cv;
using namespace std;

const int width = 270;
const int height = 75;

int iLowH = 6;
int iHighH = 20;

int iLowS =84; 
int iHighS = 255;

int iLowV = 64;
int iHighV = 255;

Mat imgOriginal;
VideoCapture cap(0); //capture the video from webcam
ofstream myfile;

void getInitPosition(int & x, int & y) {
  x = y =0;
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

int main( int argc, char** argv )
{
  cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  cap.set(CV_CAP_PROP_FPS, 200);
  myfile.open ("out.txt");

  if ( !cap.isOpened() )  // if not success, exit program
    {
      cout << "Cannot open the web cam" << endl;
      return -1;
    } 

  int x, y;
  getInitPosition(x, y);

  for (int i  = 0; i < 1000; ++i) {
    cap.read(imgOriginal);
    Mat img = imgOriginal(Rect(max(x - 10, 0), max(y - 10, 0), min(width - x, 20), min(height - y, 20)));
    
    Mat imgHSV;
    //Mat imgLines = Mat::zeros( imgOriginal.size(), CV_8UC3);

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
    }
    x = max(x - 10, 0) + newx;
    y = max(y - 10, 0) + newy;
    
    myfile << x << "\t" << y << endl;
    // circle(imgLines, Point(x, y), 5, Scalar(0,0,255), 3);    
    // imgOriginal = imgOriginal + imgLines;
    // imwrite( "image" + to_string(i) + ".png", imgOriginal);
  }
  myfile.close();
  return 0;
}
