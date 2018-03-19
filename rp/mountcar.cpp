#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include "hdw.h"
#include "udpClient.h"

using namespace std;

void * rlso;
void * initFunc;
void * rlAlgFunc;
void * cleanUpFunc;

int init(double l, double ldot) {
  return reinterpret_cast<int (*)(double, double)>(initFunc)(l, ldot);
}

int rl_alg(double l, double ldot, double r, int is_terminal) {
  return reinterpret_cast<int (*)(double, double, double, int)>(rlAlgFunc)(l, ldot, r, is_terminal);
}

int clean_up() {
  reinterpret_cast<void (*)()>(cleanUpFunc)();
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


int main( int argc, char** argv )
{
  char * error;
  rlso = dlopen("/home/pi/mountcar/rp/librlalg.so", RTLD_NOW);
  if (!rlso) {
    printf("%s\n", dlerror());    
    printf("error\n");
  }
  initFunc= (void *)dlsym(rlso, "init");
  if ((error = dlerror()) != NULL) {
      fprintf(stderr, "%s\n", error);
  }
  rlAlgFunc= (void *)dlsym(rlso, "rl_alg");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
  }
  cleanUpFunc= (void *)dlsym(rlso, "clean_up");  
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
  }
  int goal = -80;
  int totalTime = 100000;
  int maxPwm = 30;

  Mat imgOriginal;
  VideoCapture cap(0); //capture the video from webcam
  ofstream myfile;

  setpriority(PRIO_PROCESS, 0, -20);
  initPwm();
  initCam(cap);
  initUdpClient();
  
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
  bool dir = true;
  int counter = 0;

  srand(time(NULL));
  bool halt = false;
  int halt_counter = 0;
  double r;
  int a, is_terminal;
  double retVal = 0;
  double totalRet = 0;
  double epnum = 0;
  double l, ldot;

  kalman(1.07 * x - 394 - 16, l, ldot);

  a = init(l, ldot);
  a==1 ? goRight(maxPwm) : goLeft(maxPwm);

  for (int i  = 0; i < totalTime; ++i) {
    long temp = getTime();
    processImage(cap, x, y, true, false);    
    kalman(1.07 * x - 394 - 16, l, ldot);

    if (halt) {
      stop();
      if (halt_counter == 250) {
    	cout <<  "HALT  " << flush;
    	halt_counter = 0;
	epnum++;
    	totalRet = retVal;
    	retVal = 0;
    	halt = false;
      } else {
    	++halt_counter;
	if (halt_counter%11==10)
	  sendUdpData({l, (double)dir, (double)(halt), epnum, totalRet});
      }
    } else if (counter == 10) {
      is_terminal= (l <= goal);
      if (is_terminal) {
	halt = true;
	cout << "** goal ***  " << flush;
	r = 0;
      }	else {
	r = (-1)/1000.0;
      }
      retVal += r;
      a = rl_alg(l, ldot, r, is_terminal);
      dir = (a==1);
      counter = 0;
      sendUdpData({l, (double)dir, (double)(halt), epnum, totalRet});
    } else {
      ++counter;
    }
    if (!halt) a==1 ? goRight(maxPwm) : goLeft(maxPwm);
  }
  
  stop();
  clean_up();
  dlclose(rlso);
  return 0;  
}
