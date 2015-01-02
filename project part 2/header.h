/*===================================================================================================
 *  Group 18 - E/10/049, E/10/170 - Project II - Part II
 ====================================================================================================*/
 
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define MAXINT 0x7fffffff
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define BUFFFER_SIZE 100000000
#define PACKET_SIZE 100
#define WINDOW_SIZE 5
#define ERROR_RATIO 0.5
#define SEQ_NO 8

double URandom(uint32_t *uPtr, double max);
int setsocket();
void tv_sub(struct  timeval *out, struct timeval *in);
