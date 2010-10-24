/*****************
 * Ryan G Hunter
 * Gesture Protocol 04 HEADER
 * Boston University IML
 * ***************/

#include "tracker2.h"

int checkMessages(int *socket, struct packet *input)
{
	memset(input->buffer, 0, 100);
	input->len = recvfrom(*socket, input->buffer, 100, MSG_DONTWAIT, NULL, NULL);
	return 0;
}
int checkStateChange(struct packet *input,enum state *currState)
{
	if(input->len > 0 && input->buffer[0] == 'C')
	{	
		if(strcmp(input->buffer,"CSTART") == 0)
			*currState=START;
		if(strcmp(input->buffer,"CEXIT") == 0)
			*currState=EXIT;
		if(strcmp(input->buffer,"CSTOP") == 0)
			*currState=STOP;
	}
	return 0;
}
int checkTrackChange(struct packet *input,enum track *currTarg)
{
	if(input->len > 0 && input->buffer[0] == 'T')
	{	
		if(strcmp(input->buffer,"TCIRCLE") == 0)
			*currTarg = CIRCLE;
		if(strcmp(input->buffer,"TFIGEIGHT") == 0)
			*currTarg = FIGUREIGHT;
		return 1;
	}
	return 0;
}

void wait(int sec, int usec)
{
	struct timeval timeOut;
	timeOut.tv_sec= sec;
	timeOut.tv_usec= usec;
	select((int)NULL, NULL, NULL, NULL, &timeOut);
	return;
}

void cTimerStart(struct timeval *epoch)
{
	gettimeofday(epoch,NULL);
	return;
}

double cTimerElapsed(struct timeval *epoch)
{
	struct timeval curr;
	gettimeofday(&curr,NULL);
	double time = (double)((curr.tv_sec-epoch->tv_sec)+(curr.tv_usec-epoch->tv_usec)/1000000.0);
	return time;
}
int initSocket(int *sock, int port)
{
	*sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  	struct sockaddr_in sin;
  	if(*sock == -1)
	{
    		perror("Error creating socket\n");
		return 0;
	}
  	sin.sin_family = AF_INET;
  	sin.sin_port = htons(port);
  	sin.sin_addr.s_addr = htonl(INADDR_ANY);
  	if(bind(*sock, (struct sockaddr *)&sin, sizeof(sin))!=0) 
	{
    		perror("Unable to Bind");
		return 0;
	}
	return 1;
}

int updatePosition(struct robot * vehicle, struct packet * packet)
{
	int id; //,len;
	int sz = sizeof(unsigned int);
	int pktsz = 1+4*sz;

  	if(packet->buffer != NULL &&  packet->len == pktsz && packet->buffer[0] == 'R')
	{
  		int idx = 1;

		unsigned int val;
 		memcpy(&val, &packet->buffer[idx], sz);
 		id = ntohl(val);
  		idx += sz;
#ifdef DEBUG
		printf("Picked up: %i\n",id);
#endif
		if(vehicle->id==id)
		{
			memcpy(&val, &packet->buffer[idx], sz);
 			vehicle->timestamp = ntohl(val);
 			idx += sz;

			memcpy(&val, &packet->buffer[idx], sz);
 			vehicle->y = (int)(640-ntohl(val));
 			idx += sz;

 			memcpy(&val, &packet->buffer[idx], sz);
 			vehicle->x = (int)(480-ntohl(val));
#ifdef DEBUG
			printf("got Data: %i %i\n",vehicle->x,vehicle->y);
#endif
			return 1;
		}
	}
	return 0;
}
float dotProd(struct vector * v1, struct vector * v2)
{
	return (v1->i*v2->i+v1->j*v2->j);
}
float deltaDist(struct robot * mag,struct robot * origin)
{
	struct vector v3;
	v3.i = (float)(mag->x-origin->x);
	v3.j = (float)(mag->y-origin->y);
	return (float)sqrt(pow(v3.i,2)+pow(v3.j,2));
}
double vMag(struct vector * v)
{
	return sqrt(v->i*v->i+v->j*v->j);
}
