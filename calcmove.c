/*
 * File calcmove.c
 * Author: arhimed@gmail.com
 * Date 20070721
 *
 * TODO: support jerk (marlin)
 * TODO: support different limits on XYZE
 * TODO: reorganize code, group reusable segments
 *
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "calcmove.h"


void recordCoordToMinMax3D(minmax3d_t* s, double x, double y, double z) {
  s->min_x = fmin(x, s->min_x);
  s->min_y = fmin(y, s->min_y);
  s->min_z = fmin(z, s->min_z);
  s->max_x = fmax(x, s->max_x);
  s->max_y = fmax(y, s->max_y);
  s->max_z = fmax(z, s->max_z);
  // fprintf(stdout, "%f\n", z);
}


double calcmove(char * buffer,  print_settings_t * print_settings, bool resetCoords) {
  static double oldx = 0;        //old values required for sticky paremeters and other calcs
  static double oldy = 0;
  static double oldz = 0;
  static double olde = 0;
  static double oldf = 1;        //default feed rate = 1 overriden by first F in the code, //TODO: should be set from config, not hardcoded

  static double oldxa    = 0;    //vector of previous movement
  static double oldya    = 0;
  static double oldza    = 0;

  static double oldspeed = 0;    //speed previous move ended (start speed for current move)
  static double totalExtrusion = 0;


  double ret;
  double distance;
  double costheta;
  double xa,ya,za;
  double sin_theta_d2;
  double speed ;
  double x,y,z,e,f;
  char   c;
  double Ta,Td,Sa,Sd,Ts;
  bool isMoving = false;
  bool isExtruding = false;
  bool isCoordsReset = false;

  speed = 0;
  char * buff;
  buff = buffer;

  //sticky
  f = oldf;
  x = oldx;
  y = oldy;
  z = oldz;
  e = olde;

  while('\0' != (c = *(buff++)) ){
    bool endofline;
    endofline = false;
    if ((c & 0xDF) == 'G'){
      while('\0' != (c = *(buff++)) ){
        switch (c){
          case 'X':
          case 'x':
            x = atof(buff++);
            if (!print_settings->abs) {
              x += oldx;
            }
            isMoving = true;
            if (resetCoords) {
              oldx = 0;
              x = 0;
              isCoordsReset = true;
            }
            break;
          case 'Y':
          case 'y':
            y = atof(buff++);
            if (!print_settings->abs) {
              y += oldy;
            }
            isMoving = true;
            if (resetCoords) {
              oldy = 0;
              y = 0;
              isCoordsReset = true;
            }
            break;
          case 'Z':
          case 'z':
            z = atof(buff++);
            if (!print_settings->abs) {
              z += oldz;
            }
            isMoving = true;
            if (resetCoords) {
              oldz = 0;
              z = 0;
              isCoordsReset = true;
            }
            break;
          case 'E':
          case 'e':
            e = atof(buff++);

            if (resetCoords) {
              olde = 0;
              e = 0;
              isCoordsReset = true;
            }

            if (e > 0.0f) {
              isExtruding = true;
            }

            if (!print_settings->eabs) {
              totalExtrusion += e;
              e += olde;
            } else {
              totalExtrusion += e - olde;
            }
            break;
          case 'F':
          case 'f':
            f = atof(buff++)/60.0; //convert speed to units/second from units/minute
            break;
          case ';':
          case '\n':
          case '\r':
            endofline = true;
        }
        if (endofline) break;
      }
      break;
    }
  }

  if (resetCoords && !isCoordsReset) {
    oldx = x = 0;
    oldy = y = 0;
    oldz = z = 0;
    olde = e = 0;
  }

  print_settings->max_extrusion = fmax(print_settings->max_extrusion, totalExtrusion);

  ret = 0.0;
  distance = sqrt(pow((x - oldx),2) + pow((y-oldy),2) + pow((z-oldz),2));
  if (distance == 0.0){
    oldf = f;
    olde = e;
    return 0;
  }

  if (!print_settings->mm) distance = distance * 25.4; //if units are in inch then convert to mm


  xa = x - oldx;
  ya = y - oldy;
  za = z - oldz;

  if (print_settings->jerk) {
	  //MARLIN - JERK & ACCELERATION

	  ret = 0; //TODO: implement
  } else {
	  //SMOOTHIEWARE - JUNCTION DEVIATION & ACCELERATION

		costheta = (xa * oldxa + ya * oldya + za * oldza) / (sqrt(xa * xa + ya * ya + za * za) * sqrt(oldxa * oldxa + oldya * oldya + oldza * oldza));
		if (costheta < 0.95) {
			speed = f;
			if (costheta > -0.95F) {
				sin_theta_d2 = sqrt(0.5 * (1.0 - costheta));
				speed = _MIN_(speed, sqrt( print_settings->accel * print_settings->jdev * sin_theta_d2 / (1.0 - sin_theta_d2)));
			}
		}
		speed = _MIN_(speed, print_settings->x_maxspeed /60.0);
		speed = _MIN_(speed, print_settings->y_maxspeed /60.0);

		Ta = (f - oldspeed) / print_settings->accel; // time to reach speed from start speed (end speed of previous move)
		Td = (f - speed) / print_settings->accel; // time to decelerate to "end speed" of current movement
		Sa = (f + oldspeed) * Ta / 2.0;    // length moved during acceleration
		Sd = (f + speed) * Td / 2.0;       // length moved during deceleration
		if ((Sa + Sd) < distance) {
			Ts = (distance - (Sa + Sd)) / f; // time spent during constant speed
		} else {
			Ts = 0; // no time for constant speed, move was too short for accel + decel + constant speed
			        //split accel and decel in half, no clue how each firmware handles this but this seems like a proper thing to do
			        //Sa = Sd == distance/2
			        //T = 2*S / (V0+V1)
			Ta = distance / (f + oldspeed);
			Td = distance / (f + speed);
		}

		ret = Ta + Ts + Td;
		oldspeed = speed;
	}

  if (isMoving && isExtruding) {
    recordCoordToMinMax3D(&print_settings->minmax3d, x, y, z);
  }

	oldx = x;
	oldy = y;
	oldz = z;
	olde = e;
	oldf = f;

	oldxa = xa;
	oldya = ya;
	oldza = za;

	return ret;
}
