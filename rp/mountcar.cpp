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

const int NUM_STATES = 2;
const int FORCE_QUANT = 2;
double STATE_BOUNDS[NUM_STATES][2];
int NUM_GRIDS;
int NUM_TILINGS;

double EPS;
double ALPHA;
double GAMMA;
double EPISODE_MAX_STEPS;

int NUM_FEATS;
double GRID_WIDTH[NUM_STATES];
double ** TILING_OFFSETS;
int NUM_ACTS;
double ** params;

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

int ipow(double base, int exp)
{
  int result = 1;
  while (exp)
    {
      if (exp & 1)
	result *= base;
      exp >>= 1;
      base *= base;
    }

  return result;
}

double fRand(double fMin, double fMax)
{
  double f = (double)rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

void features(double * s, int * fa) {
  int tilingIndexes [NUM_TILINGS][NUM_STATES];
  for (int i = 0; i< NUM_TILINGS; i++) 
    for (int j = 0; j< NUM_STATES; j++) 
      tilingIndexes[i][j] = fmin(fmax((int)((s[j] - STATE_BOUNDS[j][0] - TILING_OFFSETS[i][j]) / GRID_WIDTH[j]), 0), NUM_GRIDS - 1);

  for (int i = 0; i< NUM_TILINGS; i++) {
    fa[i] = ipow(NUM_GRIDS, NUM_STATES) * i;
    for (int j = 0; j< NUM_STATES; j++) 
      fa[i] += ipow(NUM_GRIDS, j) * tilingIndexes[i][j];
  }
}

double Q(int * fa, int a){
  double * tmp = params[a];
  double  sum = 0;
  for(int i = 0; i < NUM_TILINGS; i++)
    sum += tmp[fa[i]];
  return sum;
}

void greedy_policy(double * s, int * a, double * v){
  double q_a[NUM_ACTS];
  int fa[NUM_TILINGS];
  features(s, fa);
  double value = -DBL_MAX;
  int indecies[NUM_ACTS];
  int indLen = 0;
  for (int i=0; i<NUM_ACTS; i++) {
    double tmp = Q(fa, i);
    if (tmp > value) {
      value = tmp;
      indLen = 0;
      indecies[indLen++] = i;
    } else if (tmp == value)
      indecies[indLen++] = i;
  }
  *a = indecies[rand() % indLen];
  *v = value;
}

void eps_policy(double * s, int * a, double * v){   
  int greedy_action;
  double value;
  greedy_policy(s, &greedy_action, &value);
  if (rand() < (int)(EPS * RAND_MAX)) {
    *a = rand() % NUM_ACTS;
    *v = value; // dummy value not correct though
  } else {
    *a = greedy_action;
    *v = value;
  }  
}

void doIter() {
  double retArray[NUM_STEPS/DISP_STEPS];
  int retArrCnt = 0;

  srand (time(NULL) * root.get<int>("tcp_port"));
  int step_loop = 1;
  int ep_loop = 1;
  int episodes = 0;
  int steps = 0;

  while (ep_loop) {
    int step_loop = 1;
    int episode_steps = 0;
    double s[NUM_SYS_STATES];
    initConds(s);
    while (step_loop) {
      steps ++;
      int a;
      double Q_a;
      eps_policy(s, &a, &Q_a);
      episode_steps ++;
      int fa[NUM_TILINGS];
      features(s, fa);
      double sp[NUM_SYS_STATES];
      double r;
      int is_terminal;
      transition(s, a, sp, &r, &is_terminal);
      double delta = r - Q(fa, a);
      double * tmp = params[a];
      if (is_terminal/* || episode_steps >= EPISODE_MAX_STEPS*/) {
	episodes ++;
	for(int i = 0; i < NUM_TILINGS; i++)
	  tmp[fa[i]] += ALPHA * delta;
	step_loop = 0;
      } else {
	int ap;
	double Q_ap;
	greedy_policy(sp, &ap, &Q_ap);
	delta += GAMMA * Q_ap;
	for(int i = 0; i < NUM_TILINGS; i++)
	  tmp[fa[i]] += ALPHA * delta;
	memcpy(s, sp, NUM_SYS_STATES * sizeof(double));
      }
      if ((steps % DISP_STEPS) == 0) {
	double initS [NUM_SYS_STATES];
	initConds(initS);
	double R = generate_episode_return(initS, pFile, steps/DISP_STEPS > 0.95 * NUM_STEPS /DISP_STEPS);
	retArray[retArrCnt++] = R;
	if ((steps % (DISP_STEPS*10)) == 0) {
	  printf("%.0f %%\n",(double)steps/NUM_STEPS*100);
	  fflush(stdout);
	}
      }
      if (steps >= NUM_STEPS) {
	step_loop = 0;
	ep_loop = 0;
      }
    }			      
  }  

  FILE * pFile;
  char str[20];
  sprintf(str, "returns.txt")
  pFile = fopen (str, "w");
  for (int i=0; i< NUM_STEPS/DISP_STEPS; i++)
    fprintf(pFile, "%f\n", retArray[i]);

  fclose (pFile);  
}

void cleanUp(){
  for(int i = 0; i < NUM_TILINGS; ++i) {
    delete [] TILING_OFFSETS[i];
  }
  delete [] TILING_OFFSETS;
  for(int i = 0; i < NUM_ACTS; ++i) {
    delete [] params[i];
  }
  delete [] params;
}

void init() {
  NUM_GRIDS = 10;
  NUM_TILINGS = 10;
  TILING_OFFSETS = new double*[NUM_TILINGS];
  for(int i = 0; i < NUM_TILINGS; ++i) {
    TILING_OFFSETS[i] = new double[NUM_STATES];
  }
  NUM_ACTS = FORCE_QUANT;

  double temp[NUM_STATES][2] = {{-150, 150}, {-1200, 1200}};
  memcpy(STATE_BOUNDS, temp, NUM_STATES*2*sizeof(double));

  EPS = 0;
  ALPHA = 0.05;
  GAMMA = 1;
  EPISODE_MAX_STEPS = 50000;
  
  NUM_FEATS = ipow(NUM_GRIDS, NUM_STATES) * NUM_TILINGS;
  for (int i=0; i<NUM_STATES; i++) {
    GRID_WIDTH[i] = (double)(STATE_BOUNDS[i][1] - STATE_BOUNDS[i][0]) / NUM_GRIDS;
  }
  srand (1);
  for (int i=0; i<NUM_TILINGS; i++)
    for (int j=0; j<NUM_STATES; j++)
      TILING_OFFSETS[i][j] = -fRand(-GRID_WIDTH[j] / 2, GRID_WIDTH[j]/ 2);

  params = new double*[NUM_ACTS];
  for(int i = 0; i < NUM_ACTS; ++i) {
    params[i] = new double[NUM_FEATS];
    memset(params[i], 0, NUM_FEATS*sizeof(double));
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

  init();
  doIter();
  cleanUp();
  

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
