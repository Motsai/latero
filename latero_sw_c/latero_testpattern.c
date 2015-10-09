// this is ported from parallel port example code: latero.c, latero.h and main.c
// TODO: to be merged together

#include <stdio.h>
#include <stdlib.h>
//#include "latero.h"
#include <math.h>
#include <sys/time.h>

#include "lat_client_api.h"
#include "latero_testpattern.h"

static double pattern[LATERO_NB_PINS];
static double mask[LATERO_NB_PINS];

int latero_write_ether(latero_conn* platero, double frame[LATERO_NB_PINS])
{
	unsigned char raw[LATERO_NB_PINS];
    latero_pkt_t response;
	int i;
	for (i=0; i<LATERO_NB_PINS; ++i)
		raw[i] = (0.5-0.5*frame[i]) * 150;
	setBlades( platero, raw );
    //printf("Raw[0]=%2.2X\n",raw[0]);
    sendNormalPacket(platero,&response);
	//latero_write_raw(raw);
}

void SetMaskRow(double mask[LATERO_NB_PINS], int row)
{
	int i,j;
	for (i=0; i<LATERO_NB_PINS_X; ++i)
		for (j=0; j<LATERO_NB_PINS_Y; ++j)
			mask[j*LATERO_NB_PINS_X+i] = (j==row);
}

void SetMaskCol(double mask[LATERO_NB_PINS], int col)
{
	int i,j;
	for (i=0; i<LATERO_NB_PINS_X; ++i)
		for (j=0; j<LATERO_NB_PINS_Y; ++j)
			mask[j*LATERO_NB_PINS_X+i] = (i==col);
}

void SetMaskAll(double mask[LATERO_NB_PINS])
{
	int i;
	for (i=0; i<LATERO_NB_PINS; ++i)
		mask[i] = 1;
}

void SetMaskNone(double mask[LATERO_NB_PINS])
{
	int i;
	for (i=0; i<LATERO_NB_PINS; ++i)
		mask[i] = 0;
}

void SetMaskFirstLastPins(double mask[LATERO_NB_PINS])
{
	SetMaskNone(mask);
	mask[0] = mask[LATERO_NB_PINS-1] = 1;
}

/** 
 * This function writes a test pattern to the Latero for a number of
 * seconds specified by duration_sec. The mask can be used to specify
 * which pins should be activated (1) or deactivated (0). The test pattern
 * is a sinusoidal oscillation with a frequency given by rate_hz.
 */
void RunTestPattern(int duration_sec, double rate_hz, double mask[LATERO_NB_PINS], latero_conn* platero)
{
	double sec;
	int i;
	struct timeval start_time, time;
	double frame[LATERO_NB_PINS];
	
	gettimeofday(&start_time, NULL);
	do
	{
		double v;
		
		/* get time since start in seconds */
		gettimeofday(&time, NULL);
		sec = (time.tv_sec-start_time.tv_sec) + (time.tv_usec-start_time.tv_usec)/1E6;
		
		/* compute oscillation */
		v = sin(2*M_PI*sec*rate_hz);
		
		/* write to Latero */
		for (i=0; i<LATERO_NB_PINS; ++i)
			frame[i] = v * mask[i];
		latero_write_ether(platero, frame);
		
	}
	while (sec < duration_sec);
}

/**
 * This functions gradually moves the pins to the positions speficied by
 * pattern, holds the activation for duration_sec, and brings the pins
 * back to rest.
 */
void RunFixedPattern(int duration_sec, double pattern[LATERO_NB_PINS], latero_conn* platero)
{
	double sec;
	int i;
	struct timeval start_time, time;
	double frame[LATERO_NB_PINS];
	double ramp_sec = 0.5;
	
	/* move to target position in ramp_sec seconds */
	gettimeofday(&start_time, NULL);
	do
	{
		/* get time since start in seconds */
		gettimeofday(&time, NULL);
		sec = (time.tv_sec-start_time.tv_sec) + (time.tv_usec-start_time.tv_usec)/1E6;
		
		/* interpolate and write */
		for (i=0; i<LATERO_NB_PINS; ++i)
			frame[i] = sec/ramp_sec * pattern[i];
		latero_write_ether(platero, frame);
	}
	while (sec < ramp_sec);
	
	/* hold for a number of seconds */
	for (i=0; i<LATERO_NB_PINS; ++i)
		frame[i] = pattern[i];
	latero_write_ether(platero,frame);
	sleep(duration_sec);
	
	/* move to back to zero position in ramp_sec seconds */
	gettimeofday(&start_time, NULL);
	do
	{
		/* get time since start in seconds */
		gettimeofday(&time, NULL);
		sec = (time.tv_sec-start_time.tv_sec) + (time.tv_usec-start_time.tv_usec)/1E6;
		
		/* interpolate and write */
		for (i=0; i<LATERO_NB_PINS; ++i)
			frame[i] = (1-sec/ramp_sec) * pattern[i];
		latero_write_ether(platero, frame);
	}
	while (sec < ramp_sec);
}

int TestSplit1(latero_conn* platero)
{
	int i,j;

	printf("Moving top half to the right and bottom half to the left for 5 seconds\n");
	for (i=0; i<LATERO_NB_PINS_X; ++i)
		for (j=0; j<LATERO_NB_PINS_Y; ++j)
			pattern[j*LATERO_NB_PINS_X+i] = (j<LATERO_NB_PINS_Y/2)?+1.0:-1.0;
	RunFixedPattern(5, pattern,platero);
}

int TestSplit2(latero_conn* platero)
{
	int i,j;
	
	printf("Moving top half to the left and bottom half to the right for 5 seconds\n");
	for (i=0; i<LATERO_NB_PINS_X; ++i)
		for (j=0; j<LATERO_NB_PINS_Y; ++j)
			pattern[j*LATERO_NB_PINS_X+i] = (j<LATERO_NB_PINS_Y/2)?-1.0:+1.0;
	RunFixedPattern(5, pattern,platero);
}

int TestFirstlast(latero_conn* platero)
{
	int i,j;
	
	printf("Running test pattern on first and last pins: upper-left and lower-right corners (5 seconds at 10 Hz).\n");
	SetMaskFirstLastPins(mask);
	RunTestPattern(5,10,mask,platero);
}

int TestAllpin(latero_conn* platero)
{
	int i,j;
	
	printf("Running test pattern on all pins (10 seconds at 30 Hz).\n");
	SetMaskAll(mask);
	RunTestPattern(10,30,mask,platero);	
}

int TestRow(latero_conn* platero)
{
	int i,j;
	for (i=0; i<LATERO_NB_PINS_Y; ++i)
	{
		printf("Running test pattern on row %d (2 seconds at 40 Hz).\n", i);
		SetMaskRow(mask,i);
		RunTestPattern(1,40,mask,platero);
	}
}

int TestCol(latero_conn* platero)
{
	int i,j;
	for (i=0; i<LATERO_NB_PINS_X; ++i)
	{
		printf("Running test pattern on column %d (2 seconds at 50 Hz).\n", i);
		SetMaskCol(mask,i);
		RunTestPattern(1,40,mask,platero);
	}
}


int TestInit()
{
	printf("Please orient the Latero such that its cable is to the left.\n");
	sleep(1);
	
}

