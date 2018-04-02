/* fpont 12/99 */
/* pont.net    */
/* udpClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */
#include <time.h>

//for Mac OS X
#include <stdlib.h>
#include "udpClient.h"
#include <iostream>

#define REMOTE_SERVER_PORT 1500
#define REMOTE_SERVER_ADDR "cps.ics.uci.edu"
#define MAX_MSG 100

struct sockaddr_in cliAddr, remoteServAddr;
int sock;

unsigned long long getEpoch()
{
  struct timeval start;
  long seconds, useconds;

  gettimeofday(&start, NULL);
  seconds  = start.tv_sec;
  useconds = start.tv_usec;

  return (unsigned long long)(seconds) * 1000 + (unsigned long long)useconds/1000;
}
int initUdpClient() {
  int rc;
  struct hostent *h;
  int broadcast = 1;

  
  h = gethostbyname(REMOTE_SERVER_ADDR);
  if(h==NULL) {
    printf("unknown host");
    exit(1);
  }

  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr, 
	 h->h_addr_list[0], h->h_length);
  remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

  /* socket creation */
  sock = socket(AF_INET,SOCK_DGRAM,0);
  if(sock<0) {
    printf("cannot open socket \n");
    exit(1);
  }

  
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof broadcast) == -1) {
    perror("setsockopt (SO_BROADCAST)");
    exit(1);
  }

  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);
  

  rc = bind(sock, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if(rc<0) {
    printf("cannot bind port\n");
    exit(1);
  }
}
int sendUdpData(vector<double> data) {
  int rc;


  /* send data */
  rc = sendto(sock, "start", strlen("start")+1, 0, 
	      (struct sockaddr *) &remoteServAddr, 
	      sizeof(remoteServAddr));
  unsigned long long ms = getEpoch();
  char str[50];
  snprintf(str, 50, "%llu", ms);
  rc = sendto(sock, str, strlen(str)+1, 0, 
	      (struct sockaddr *) &remoteServAddr, 
	      sizeof(remoteServAddr));
  
  for (auto entry: data) {
    char str[50];
    snprintf(str, 50, "%f", entry);
    
    rc = sendto(sock, str, strlen(str)+1, 0, 
		(struct sockaddr *) &remoteServAddr, 
		sizeof(remoteServAddr));
  }
  
  return 1;
}
