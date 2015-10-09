#ifndef LATERO_TESTPATTERN_H 
#define LATERO_TESTPATTERN_H


// this is ported from parallel port example code: latero.c, latero.h and main.c
// TODO: to be merged together

#define LATERO_NB_PINS_X 8
#define LATERO_NB_PINS_Y 8
#define LATERO_NB_PINS (LATERO_NB_PINS_X*LATERO_NB_PINS_Y)

//Moving top half to the right and bottom half to the left for 5 seconds
int TestSplit1(latero_conn* platero);

//Moving top half to the left and bottom half to the right for 5 seconds
int TestSplit2(latero_conn* platero);

//Running test pattern on first and last pins: upper-left and lower-right corners (5 seconds at 10 Hz)
int TestFirstlast(latero_conn* platero);

//Running test pattern on all pins (10 seconds at 30 Hz)
int TestAllpin(latero_conn* platero);

//Running test pattern on rows
int TestRow(latero_conn* platero);

//Running test pattern on columns
int TestCol(latero_conn* platero);

//	Please orient the Latero such that its cable is to the left.
int TestInit();

#endif