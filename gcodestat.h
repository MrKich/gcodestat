/*
 * gcodestat.h
 *
 *  Created on: Dec 2, 2017
 *      Author: arhimed
 */

#ifndef GCODESTAT_H_
#define GCODESTAT_H_

#include <stdbool.h>
#include <stdio.h>

#define LINE_BUFFER_LENGTH         4096
#define DEFAULT_ACCELERATION       1000
#define DEFAULT_JUNCTION_DEVIATION    0.02
#define DEFAULT_JERK                  4.00
#define DEFAULT_MAX_SPEED        100000
#define DEFAULT_RETRACT_TIME          0.0
#define DEFAULT_PRIME_TIME            0.0

#define GCODE_IRRELEVANT 0
#define GCODE_MOVE       1
#define GCODE_DWELL      2
#define GCODE_RETRACT    3
#define GCODE_INCH       4
#define GCODE_MM         5
#define GCODE_ABS        6
#define GCODE_REL        7
#define GCODE_EABS       8
#define GCODE_EREL       9
#define GCODE_MAXFEED   10
#define GCODE_ACCEL     11
#define GCODE_JDEV      12
#define GCODE_RLEN      13
#define GCODE_PLEN      14
#define GCODE_SPEEDOVER 15
#define GCODE_RESET_COORD 16

typedef struct {
  double min_x;
  double min_y;
  double min_z;
  double max_x;
  double max_y;
  double max_z;
} minmax3d_t;

//TODO: make X,Y,Z accel/jdev separately configured
typedef struct {
	double accel;
	double jdev;
	double x_maxspeed;
	double y_maxspeed;
	double z_maxspeed;
	double rtime;
	double ptime;
	double speedoverride;
	bool abs;
	bool eabs;
	bool mm;
	bool jerk;
	bool output_seconds;
	minmax3d_t minmax3d;
	double max_extrusion;
} print_settings_t;

void initMinMax3D(minmax3d_t* s);
void printDimensions(FILE* f, minmax3d_t* s);

#endif /* GCODESTAT_H_ */
