// -----------------------------------------------------------
//
// Copyright (c) 2015 by Tactile Labs Inc. All Rights Reserved.
//
// Author : Jean-Samuel Chenard (chenard@tactilelabs.com)
//
// This file is part of latero-graphics.
//
//    latero is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    latero is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with latero.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------

#ifndef LATERO_H
#define LATERO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include "latero_io.h"

#define LATERO_NB_PINS_X 8
#define LATERO_NB_PINS_Y 8
#define LATERO_NB_PINS (LATERO_NB_PINS_X*LATERO_NB_PINS_Y)

// number of counts per rotation of encoders
#define ENCODER_CPR 65536

// @todo verify that this value is correct 
#define LATERO_MAX_RAW_PIN 150

// full workspace size, not taking into account disc
#define FULL_WORKSPACE_WIDTH 381
#define FULL_WORKSPACE_HEIGHT 317.5

// diameter of disc under Latero
#define DISC_DIAMETER 100

#define WORKSPACE_WIDTH (FULL_WORKSPACE_WIDTH-DISC_DIAMETER)
#define WORKSPACE_HEIGHT (FULL_WORKSPACE_HEIGHT-DISC_DIAMETER)

// bit masks for button 0 (left) and button 1 (right) in the DIO_IN reading
// value is 0 when pressed, 1 otherwise
#define LATERO_BUTTON0_MASK 0x0040
#define LATERO_BUTTON1_MASK 0x0020
    
/* Opaque structure that defines a connection with the server 
   Maintains a set of states and buffers.  Elements of this structure
   should only be modified by this API, not directly by the client.
*/
typedef struct
{
  char   initialized;
  int    udp_socket;   
  struct sockaddr_in si_server;
  char   pktbuff[BUFLEN];
  char   rspbuff[BUFLEN];
  uint16_t dac[4];
  uint8_t  pins[64];
  uint16_t dio_out;
  int encoder_offset[3]; // offset of encoder to 0 degrees 
} latero_t;


/**
 * Destination for raw read/write operations. (ADVANCED)
 */
typedef enum x { LATERO_CONTROLLER, LATERO_IO } latero_dst_device;


/* 
 * Open connection to the Latero.
 * @return 0 on success, negative on failure
 */
int latero_open(latero_t* latero, const char* str_ip_address);


/*
 * Close connection to the Latero.
 * @return 0 on success, negative on failure 
 */
int latero_close(latero_t* latero);


/**
 * @return 1 if open, 0 otherwise.
 */
int latero_is_open(latero_t* latero);
                   

/**
 * Sets the raw values (byte) for the latero pins.
 * @param values  array of LATERO_NB_PINS byte values 
 * @warning not effective until next write to device
 * @TODO specify valid range and direction
 */
void latero_set_pins_raw(latero_t* latero, char* values);


/**
 * Sets the values of the latero pins (-1.0 to 1.0)
 * @param blade_values  array of LATERO_NB_PINS floating point values (-1.0 to 1.0) 
 * @warning not effective until next write to device
 * @TODO specify direction
 */
void latero_set_pins(latero_t* latero, double frame[LATERO_NB_PINS]);


/**
 * Set analog output value at a given index. (ADVANCED)
 * @param index  DAC index
 * @param value  DAC value
 * @warning not effective until next write to device
 * @TODO specify valid range of index and value
 */
void latero_set_DAC(latero_t* latero, char index, uint16_t value);


/**
 * Set DIO. (ADVANCED)
 * @warning not effective until next write to device
 * @TODO specify meaning of value
 */ 
void latero_set_DIO(latero_t* latero, uint16_t value);


/**
 * Write currently set state to the Latero. This includes pins, DAC and DIO values. 
 * Use SET functions to set these values before calling latero_write.
 * @param response  response packet returned by Latero (optional, can be set to NULL)
 * @return TODO
 */
int latero_write(latero_t* latero, latero_pkt_t* response);


/**
 * Write a frame of blade values to the Latero. Combines set and write operations.
 * @param response  response packet returned by Latero (optional, can be set to NULL) 
 */
int latero_write_pins(latero_t* latero, double frame[LATERO_NB_PINS], latero_pkt_t* response);


/**
 * Write DIO. Combines set and write operations. (ADVANCED)
 * @param response  response packet returned by Latero (optional, can be set to NULL) 
 */ 
int latero_write_DIO(latero_t* latero, uint16_t value, latero_pkt_t* response);


/** 
 * Raw write (ADVANCED)
 */
int latero_raw_write(latero_t* latero, latero_dst_device destination, uint16_t address, uint16_t data);


/**
 * Raw read (ADVANCED)
 */
int latero_raw_read(latero_t* latero, latero_dst_device destination, uint16_t address, uint16_t* data_read);


void latero_compute_position(latero_t *latero, uint32_t encoder_values[4], double *px, double *py, double *ptheta);
void latero_reset_position(latero_t *latero, uint32_t encoder_values[4]);

#ifdef __cplusplus
}
#endif

#endif
