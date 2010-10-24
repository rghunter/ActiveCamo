/***********
 * Camo Control Interface
 * Notes on Operation:
 * - integer pointers are for motor velocities.
 * - the return integers is an error condition- return 1 to update motors
 * - struct robot:
 *	unsigned int timestamp - from packet
 *	unsigned int id - self explanatory
 *	int x
 *	int y
 * - Gains values: 
 * 	- good speed = 10,000
	- kp (proportional) ~= 75 for standard distance tracking kp*(x1-x2)
 * turn_radius is in pixels?
 * txer_reference_x integer holding the reference x-coordinate that the transmitter robot must track.
 * Let me know if you have any further questions.
*************/

#include "tracker2.h"

/* camoControl
 *   Overlay a message on to motion along x axis. The goal is to modulate the relative separation
 * between signal and receiver while having the spy eclipsed by the receiver.
 */
int camoControl(int * leftv, int * rightv, struct robot * txer, struct robot * rxer, struct robot * spy, double turn_radius, int txer_reference_x, int input)
{
  static double xprev = 0.0;
  static double tprev = 0.0;
  double offset = 300.0;
  double scale = 150.0;
  double Kpv = 50.0;
  double Kpo = 20.0;
  double Kdo = 5.0;

  double rho = (double)sqrt( (txer->x-rxer->x)*(txer->x-rxer->x) + (txer->y-rxer->y)*(txer->y-rxer->y) );
  double sin_theta_01 = ((double)(rxer->x-txer->x))/rho;
  double cos_theta_01 = ((double)(rxer->y-txer->y))/rho;
  double cot_theta_03 = ((double)(spy->y-rxer->y))/((double)(spy->x-rxer->x));

  //double v = Kpv * ( rho * cos_theta_01 - rho * sin_theta_01 * cot_theta_03 );
  double v = (double)input;
  struct timeval curtime; //scope

  double omega = 0;

  if(fabs(txer->y-offset) <= scale) {
    double y = (txer->y - offset)/scale;
    double ysq = y*y;
    double nr = ((-1+ysq)*(-12*y*(-1+ysq) *cos(y)+(7-32*ysq+ysq*ysq)*sin(y)));
    double dr = (1+(-1+ysq)*(-1+ysq)*(-1+ysq)*(-1+ysq)*((-1+ysq)*cos(y)+6*y*sin(y))*((-1+ysq)*cos(y)+6*y*sin(y)));
    dr *= dr;
    omega = v*nr/dr;
  }/* else {
    omega = Kpo*(txer->x - txer_reference_x);
    gettimeofday(&curtime, NULL);
    if(tprev != 0.0) {
      double curt = (double)curtime.tv_sec + ((double)curtime.tv_usec)*1.0e-6;
      if(curt - tprev > 0.1) {
	omega += Kdo*(txer->x-xprev);
      }
    } 
  }*/
  //tprev = (double)curtime.tv_sec + ((double)curtime.tv_usec)*1.0e-6;
  //xprev = txer->x;

  *leftv  = (int)round(v - turn_radius*omega);
  *rightv = (int)round(v + turn_radius*omega);
  return 1;
}
