#include "tracker2.h"

#define SAMPLEDISTANCE 5
#define VEHICLE_ID 2
#define XTRACK

#define RADCONST 57.32 //180/pi


int checkObst(void);

enum state start(void);
struct sOdometryTrack ot;
struct robot currentVehicle, dummy, gotoPosition;
const int cruiseVel = 10000;
const int kP = 100;
int captureFlag = 0;
char firstRun = 1;


float distTravel(struct sOdometryTrack ot)
{
	return 100*sqrt(ot.result.x*ot.result.x+ot.result.y*ot.result.y);
}

int main(int argc, char *argv[])
{
	odometry_track_init();
	khepera3_init();
	commandline_init();
	commandline_parse(argc,(char *)argv);
	if(commandline_option_provided("-h","--help"))
	{
		printf("*******************************\n* Tracker1\n* Ryan G. Hunter\n* Boston Universtiy Intelligent Mechatronics Lab\n* \n* Parameters:\n*\n* PD Gains Settings:\n* Distance -d --distance    Pixel Distance Rho\n* Rho Gain -kRo --kRo    Distance Proportional gain\n* Omega Gain -kOm --kOmega    Phi (omega) Proportional Gain\n* Omega -Om --omega     Set Omega Constant\n*********************************\n");
		return 0;
	}
	struct control options;
	options.distance = commandline_option_value_int("-d", "--distance", 10); //TODO:NEED TO COME UP WITH DEFAULT
	options.kRop = commandline_option_value_float("-kRop","--kRop",75);
	options.kRod = commandline_option_value_float("-kRod","--kRod",8);
	options.kOmegaXp = commandline_option_value_float("-kOmXp","--kOmegaXp",20);
	options.kOmegaXd = commandline_option_value_float("-kOmXd","--kOmegaXd",3);
	double turn_radius=(double)commandline_option_value_float("-tRad","--TurnRadius",1.0);
	double txer_reference_x=(double)commandline_option_value_float("-txRef","-txrReferenceX",0.0);
	khepera3_motor_initialize(&khepera3.motor_left);
	khepera3_motor_initialize(&khepera3.motor_right);
	khepera3_motor_start(&khepera3.motor_left);
	khepera3_motor_start(&khepera3.motor_right);
	int gps_sock, com_sock;
	if(!initSocket(&gps_sock,4950))
		printf("issue with socket\n");
	if(!initSocket(&com_sock,4100))
		printf("Problem with comm sock\n");
	currentVehicle.id=VEHICLE_ID;
	enum state CurrState = STOP, CurrLaw = VSPY;
	struct robot xmit,rcvr,spy;
	struct timeval epochTimer;
	xmit.id = commandline_option_value_int("-idT", "--signalerID", 1);
	rcvr.id = commandline_option_value_int("-idR", "--receiverID", 2);
	spy.id = commandline_option_value_int("-idSpy", "--spyID", 0);
	int role = commandline_option_value_int("-r", "--role", 1);

#ifdef DEBUG
	printf("Distance: %i\nkRop: %f\nkRod: %f\nkOmegaXp: %f\nkOmegaXd: %f\nTransmitter ID: %i\nRecvr ID: %i\nSpy ID: %i\nRole: %i\n",options.distance,options.kRop,options.kRod,options.kOmegaXp,options.kOmegaXd,xmit.id,rcvr.id,spy.id,role);
#endif
	switch (role)
	{
		case 1:
			CurrLaw = CAMOTRACK;
			break;
		case 2:
			CurrLaw = RECEIVE;
			break;
		case 3:
			CurrLaw = VSPY;
			break;
	}

	struct packet buffer, commands;
	int vel;

	float oldVal = 0;

	khepera3_drive_set_current_position(0,0);

	float omegaOld = 0;

	while(1)
	{
		checkMessages(&gps_sock,&buffer);
		checkMessages(&com_sock,&commands);
		khepera3_drive_get_current_speed();
#ifdef OBAVOID
		if(checkObst() && CurrState != HOLD)
			CurrState=OBDETECTED;
#endif
		checkStateChange(&commands,&CurrState);
			
		switch(CurrState)
		{
			case STOP:
				khepera3_drive_set_speed(0,0);
				CurrState=HOLD;
				break;
			case START:
				khepera3_drive_set_speed(10000,10000);
				CurrState = GETPOS;
				if(CurrLaw == RECEIVE)
					captureFlag = SPY+XMIT;
				if(CurrLaw == VSPY){
					printf("ping\n");
					captureFlag = XMIT;}
				break;
			case HOLD:		//intended to be an intermediate state when waiting for a next state from the handheld/client
				break;
			case EXIT:
				khepera3_drive_stop();
				printf("Exiting...\n");
				return 0;
				break;

#ifdef OBAVOID
			case OBDETECTED:
				khepera3_drive_set_current_position(0,0);
				khepera3_drive_get_current_position();
				long sensorBuff[11];
				irProx(sensorBuff);
				float leftAv=(sensorBuff[1]+sensorBuff[2]+sensorBuff[3])/3.0;
				float rightAv=(sensorBuff[4]+sensorBuff[5]+sensorBuff[6])/3.0;
				int turnVal = 2.11*2765 / 4;
				if(leftAv>rightAv)
				{
					leftTurn = khepera3.motor_left.current_position-turnVal;
					rightTurn = khepera3.motor_right.current_position+turnVal;

				}
				else if(leftAv<rightAv)
				{
					leftTurn = khepera3.motor_left.current_position+turnVal;
					rightTurn = khepera3.motor_right.current_position-turnVal;
				}
				else
				{
					leftTurn = khepera3.motor_left.current_position-2*turnVal;
					rightTurn = khepera3.motor_right.current_position+2*turnVal;
				}
				printf("Right Pos: %i Left Pos: %i\n",rightTurn,leftTurn);

				khepera3_drive_goto_position_using_profile(leftTurn,rightTurn);
				CurrState=TURNDELAY; //TODO: not correct Reinsertion
				break;

			case TURNDELAY:
				khepera3_drive_get_current_speed();
				if(khepera3.motor_left.current_speed < 100 && khepera3.motor_right.current_speed < 100)
				{
					khepera3_drive_set_speed(10000,10000);
					CurrState = TRACK;
					roOld = 0;
					phiOld = 0;
					break;
				}
				else
					break;
#endif


			case GETPOS:
				if(updatePosition(&xmit,&buffer))
				{
					captureFlag |= XMIT;
#ifdef DEBUG
					printf("Got XMIT\nCapture Flag: %i\n",captureFlag);
#endif
					break;
				}
				if(updatePosition(&spy,&buffer))
				{
					captureFlag |= SPY;
#ifdef DEBUG
					printf("Got SPY\nCapture Flag; %i\n",captureFlag);
#endif
					break;
				}
				if(updatePosition(&rcvr,&buffer))
				{
					captureFlag |= RCVR;
#ifdef DEBUG
					printf("Got RCVR\nCapture Flag %i\n",captureFlag);
#endif
					break;
				}
				if(captureFlag == ALL)
				{
					
#ifdef DEBUG
					printf("Got ALL\nCapture Flag: %i\n",captureFlag);
#endif
					CurrState = CurrLaw;
				}

				break;

			case CAMOTRACK:
				{
#ifdef DEBUG
				printf("\nBegin Debug for Camotrack\n");
#endif
				float y12 = (float)(rcvr.y-xmit.y), y13=(float)(spy.y-xmit.y);
				float x1 = (float)(rcvr.x-xmit.x), x2=(float)(spy.x-xmit.x);
				float proportion = options.kRop*(y12/x1-y13/(x1+x2));
				if(oldVal != 0){ vel = proportion + options.kRod*(oldVal - proportion);
				}else{ vel = proportion;}
#ifdef DEBUG
				printf("Control Law Output\ny12: %f\ny13: %f\nx1: %f\nx2: %f\nProportion: %f\n",y12,y13,x1,x2,proportion);
#endif
#ifndef XTRACK
				khepera3_drive_set_speed(vel,vel);
#endif
				captureFlag = 0x00;
				CurrState = XLOCK;
			//	oldVal = vel;
				if(xmit.y > 150)
					CurrLaw=DLAW;
				break;
				}
#ifdef XTRACK
			case XLOCK:
				{
#ifdef DEBUG
					printf("\nDebug Data for X Lock\n");
#endif
					int currX=0;
					if(role == 1){
						currX=xmit.x;}
					if(role == 2){
						currX=rcvr.x;}
					if(role == 3){
						currX=spy.x;}
					if(firstRun){
						txer_reference_x=currX;
						CurrState=GETPOS;
						firstRun=0;
						break;}

					float xError = (float)(currX-(int)txer_reference_x);
					//float omega = (omegaOld==0 ? options.kOmegaXp*xError : options.kOmegaXp*xError+options.kOmegaXd*(options.kOmegaXp*xError-omegaOld));
					float omega;
				//	if(omegaOld==0){
						omega = options.kOmegaXp*xError;
				/*	}else{
							float proportion=options.kOmegaXp*xError;
							float deriv=options.kOmegaXd*(omegaOld-proportion);
#ifdef DEBUG
							printf("Proportion: %f\nDerivative: %f\n",proportion,deriv);
#endif~/rghunter # ./activeCamo -idR 0 -idSpy 1 -idS 2  -r 2 -kOmegaXp 20

						}
						*/
					khepera3_drive_set_speed((vel-omega),(vel+omega));
					omegaOld=omega;
					CurrState = GETPOS;
#ifdef DEBUG
					printf("Omega: %f\nxError: %f\n",omega,xError);
#endif
					break;
				}
#endif

			case VSPY:
			{
#ifdef DEBUG
				printf("\nBegin VSPY Debug Info\n");
#endif
				float y23 = (float)(rcvr.y-spy.y-options.distance);
				float proportion = options.kRop*(y23);
				if(oldVal !=0){vel = proportion + options.kRod*(oldVal - proportion);
				}else{ vel = proportion;}
				//khepera3_drive_set_speed(vel,vel);
				if(proportion < 0)
					proportion = 0;
				khepera3_drive_set_speed((int)proportion,(int)proportion);
				vel = proportion;
				captureFlag = XMIT;
				CurrState = XLOCK;
				oldVal = vel;
#ifdef DEBUG
				printf("Proportion: %f\nY-Distance: %f\n",proportion,y23);
#endif
				break;
			}
			case RECEIVE:
				{
					vel = 10000;
#ifdef DEBUG
					printf("properly Entered Recieve Loop\n");
#endif
					CurrState = XLOCK;
					captureFlag = XMIT+SPY;
					break;
				}
			case DLAW:{
					int leftv,rightv;
					if(camoControl(&leftv,&rightv,&xmit,&rcvr,&spy,turn_radius,txer_reference_x,vel))
						khepera3_drive_set_speed(leftv,rightv);
					captureFlag = 0x00;
					break;
				  }	
		}
	}	
	return 0;
}	


int checkObst(void)
{
	long sensorBuffer[11];
	irProx(sensorBuffer);
	int i;
	for(i=1;i<7;i++)
	{
		if(sensorBuffer[i]<10)
		{
			printf("Detected! %i\n",i);
			return 1;
		}
	}
	return 0;
}
