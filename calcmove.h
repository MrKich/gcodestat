/*
 * calcmove.h
 * Author: arhimed
 */

#ifndef CALCMOVE_H_
#define CALCMOVE_H_

#include <stdbool.h>
#include "gcodestat.h"

#ifndef _MIN_
  #define _MIN_(a,b) (((a)<(b))?(a):(b))
#endif


double calcmove(char *,  print_settings_t *, bool resetCoords);
void recordCoordToMinMax3D(minmax3d_t* s, double x, double y, double z);


#endif /* CALCMOVE_H_ */
