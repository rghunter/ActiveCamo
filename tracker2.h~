/*****************
 * Ryan G Hunter
 * Gesture Protocol 04 HEADER
 * Boston University IML
 * ***************/

#ifndef HTRACKER2
#define HTRACKER2

//#include <korebot/korebot.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>
#include "odometry_track.h"
#include "khepera3.h"
#include "sensorGrab.h"

#define DEBUG

#define XMIT 	0x01
#define SPY 	0x02
#define RCVR	0x04
#define ALL	0x07


#define RADCONST 57.32 //180/pi

#ifdef OBAVOID
enum state{START, EXIT, STOP,HOLD,OBDETECTED,GETPOS,TURNDELAY,VSPY,CAMOTRACK,RECEIVE};
int leftTurn, rightTurn; //for the turn wait state
#else
enum state{START, EXIT, STOP,HOLD,GETPOS,VSPY,CAMOTRACK,RECEIVE,DLAW,XLOCK};
#endif
enum track{FIGUREIGHT,LINE,CIRCLE,PARTNER,NONE,POINT};

struct robot
{
	unsigned int timestamp;
	unsigned int id;
	int x;
	int y;
	enum track target;
};

struct control
{
	int distance;
	float gain;
	int timeout;
	float error;
	float kRop;
	float kRod;
	float kOmegap;
	float kOmegad;
	float omega;
	float kOmegaXp;
	float kOmegaXd;
};

struct packet
{
	char buffer[100];
	int len;
};

struct vector
{
	double i;
	double j;
};

struct point
{
	int x;
	int y;
};

double vMag(struct vector *);

int checkMessages(int *, struct packet *);
int checkStateChange(struct packet *,enum state *);
int checkTrackChange(struct packet *,enum track *);
void wait(int, int);
void cTimerStart(struct timeval *);
double cTimerElapsed(struct timeval *);
int initSocket(int *, int);
void setSpeed(int, int);
int updatePosition(struct robot *, struct packet *);
float dotProd(struct vector *, struct vector *);
float deltaDist(struct robot *,struct robot *);
void commandline_init(void);
void commandline_parse(int,char *);
int commandline_option_value_int(const char *,const char *, int);
float commandline_option_value_float(const char *,const char *, float);
int commandline_option_provided(const char *,const char *);

int camoControl(int *, int *, struct robot *, struct robot *, struct robot *, double, int,int);

#endif
