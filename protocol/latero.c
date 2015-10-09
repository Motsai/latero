// -----------------------------------------------------------
//
// Copyright (c) 2015 by Tactile Labs Inc. All Rights Reserved.
//
// Author : Jean-Samuel Chenard (chenard@tactilelabs.com)
//
// This file is part of latero.
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

#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <math.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>

#include "latero_io.h"
#include "latero.h"

#define TIMEOUTS_ENABLED

// arm lengths, root to display [mm]
#define L0 222.3
#define L1 245.6
#define L2 62.7

// offset of root relative to full workspace
#define ROOT_OFFSET_X 11.1
#define ROOT_OFFSET_Y -61.2




/***** PRIVATE API *****/

/**
 * Compute position of end effector from joint angles (see http://bit.ly/fZTaMY).
 * @param angles Array of joint angles from root to device [rad].
 * @param x,y,theta Position an orientation.
 * @warning These values are computed based on an internal frame of reference. Origin is
 *          at the first joint. Angles increase in the counterclockwise direction, with 
 *          zero towards positive x. 
 */
void latero_forward_kinematics(double angle[3], double *px, double *py, double *ptheta)
{
	*px = L0*cos(angle[0]);
	*px += L1*cos(angle[0]+angle[1]);	
	*px += L2*cos(angle[0]+angle[1]+angle[2]);
    
	*py = L0*sin(angle[0]);
	*py += L1*sin(angle[0]+angle[1]);	
	*py += L2*sin(angle[0]+angle[1]+angle[2]);
    
    *ptheta = angle[0]+angle[1]+angle[2];
}


int writeDIODir(latero_t* latero, uint16_t value )
{
    return latero_raw_write(latero, LATERO_IO, 0x18, value);
}


int readDIODir( latero_t* latero, uint16_t *value)
{
    return latero_raw_read(latero, LATERO_IO, 0x18, value);
}


int socketIsReadable( int sock_desc, int ms_timeout ) {
	
    fd_set socketReadSet;

    FD_ZERO(&socketReadSet);
    FD_SET(sock_desc,&socketReadSet);

    struct timeval timeout;

    if ( ms_timeout > 0 ) {
        timeout.tv_sec  =  ms_timeout / 1000;
        timeout.tv_usec = (ms_timeout % 1000) * 1000;
    } else {
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;
    }

    if ( select(sock_desc+1,&socketReadSet,0,0,&timeout ) < 0 ) {
        fprintf(stderr,"Socket Error on select()");
        return 0;
    }
    return ( FD_ISSET(sock_desc,&socketReadSet) != 0 );
}



void raw_cmd_packet(uint8_t command, uint16_t addr, uint16_t data, latero_pkt_t* pkt ) {
    pkt->hdr.magic   = LATERO_MAGIC_NB;
	pkt->hdr.version = PKT_VER_REV; 
	pkt->hdr.type    = PKT_TYPE_RAW; 
	pkt->raw.command = command;
	pkt->raw.address = addr;
	pkt->raw.data    = data;
}


int exchange_packet(latero_t* latero, latero_pkt_t* to_send, latero_pkt_t* response)
{
  ssize_t numbytes;
  struct sockaddr si_other;
  socklen_t slen = sizeof(si_other);

  packPacket( latero->pktbuff, BUFLEN, to_send );
  numbytes = sendto( latero->udp_socket, latero->pktbuff, BUFLEN, 0, 
                     (struct sockaddr*) &latero->si_server, 
                     sizeof(struct sockaddr) );
  if (numbytes < 0 ) {
    fprintf(stderr,"Packet sending error!\n");
    return(-1);
  }
#ifdef TIMEOUTS_ENABLED
  if( socketIsReadable( latero->udp_socket, 5 ) ) {
#endif       
    numbytes = recvfrom( latero->udp_socket, &latero->rspbuff, BUFLEN, 0, 
                         (struct sockaddr*) &si_other, &slen );
    if ( numbytes < 0 ) {
      fprintf(stderr,"Error receiving response\n");
      return(-1);
    }
#ifdef TIMEOUTS_ENABLED
  } else {
    printf("Warning! Timeout waiting from response from the Latero\n");
    printf("  - Ensure that you use the right IP to communicate with the server.\n");
    printf("  - Check that the server is running on the Latero.\n");
  }
#endif
  unpackPacket( latero->rspbuff, BUFLEN, response );
  return(0);
}


/***** PUBLIC API *****/


int latero_write(latero_t* latero, latero_pkt_t* response)
{
	int ii;
    latero_pkt_t pkt, rpkt;
    
    if (response == NULL)
        response = &rpkt;

	pkt.hdr.magic   = LATERO_MAGIC_NB;
	pkt.hdr.version = PKT_VER_REV; 
    pkt.hdr.type    = PKT_TYPE_FULL;
	pkt.full.dio_out = latero->dio_out;
    for (ii=0; ii<4; ii++)
      pkt.full.dac[ii] = latero->dac[ii];
    for (ii=0; ii<64; ii++) 
	  pkt.full.blade[ii] = latero->pins[ii];
	
    return exchange_packet(latero, &pkt, response);
}


int latero_raw_write(latero_t* latero, latero_dst_device destination, uint16_t address, uint16_t data ) 
{
  uint16_t command;
  latero_pkt_t response, request;

  command = PKT_RAW_CMD_WR;
  if ( destination == LATERO_CONTROLLER )
    command += PKT_RAW_CMD_CTRL;
  else if ( destination == LATERO_IO )
    command += PKT_RAW_CMD_IO;
  
  /* Prepare raw command request packet */
  raw_cmd_packet(command, address, data, &request);
  return exchange_packet(latero, &request, &response);
}


int latero_raw_read(latero_t* latero, latero_dst_device destination, uint16_t address, uint16_t* data_read )
{
  uint16_t command;
  latero_pkt_t response;
  latero_pkt_t request;
  int retcode;
  command = PKT_RAW_CMD_RD;
  if ( destination == LATERO_CONTROLLER )
    command += PKT_RAW_CMD_CTRL;
  else if ( destination == LATERO_IO )
    command += PKT_RAW_CMD_IO;
  
  /* Prepare raw command request packet */
  raw_cmd_packet( command, address, 0x0000, &request);
  retcode = exchange_packet( latero, &request, &response );
  /* Extract the returned data (data remotely read) */
  *data_read = response.raw.data;
  return retcode;
}


int latero_open( latero_t* latero, const char* str_ip_address )
{
  int ii;

  latero->initialized = 0;

  memset( (char *)&latero->si_server, 0, sizeof(struct sockaddr_in));
  latero->si_server.sin_family = AF_INET;
  latero->si_server.sin_port = htons(PORT);
  latero->si_server.sin_addr.s_addr = inet_addr(str_ip_address);

  memset( (char *)latero->pktbuff, 0xFF, BUFLEN );
  memset( (char *)latero->rspbuff, 0xFF, BUFLEN );
  
  for (ii=0; ii<4; ii++)
    latero->dac[ii] = 0;

  for (ii=0; ii<64; ii++)
    latero->pins[ii] = 0x80; /* Initialize to mid position */

  latero->dio_out = 0x1000; /* Now only one LED will be on */

  latero->udp_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
  if (latero->udp_socket == -1)
    return(-1);
  
  latero->initialized = 1;
  return(0);
}


int latero_close(latero_t* latero) 
{
	latero->initialized = 0;
    if ( close( latero->udp_socket ) != 0  ) 
    {
      fprintf(stderr,"Closing failed on socket\n");
      return(-1);
    }
    return( 0 );
}


void latero_set_pins_raw(latero_t* latero, char* blade_values)
{
    int ii;
    for (ii=0; ii<64; ii++)
        latero->pins[ii] = blade_values[ii];
}


void latero_set_pins(latero_t* platero, double frame[LATERO_NB_PINS])
{
	unsigned char raw[LATERO_NB_PINS];
 	int i;
	for (i=0; i<LATERO_NB_PINS; ++i)
		raw[i] = (0.5-0.5*frame[i]) * LATERO_MAX_RAW_PIN;
	latero_set_pins_raw(platero, raw);
}


void latero_set_DAC(latero_t* latero, char index, uint16_t value)
{
    assert(index < 4 && index >= 0);
    latero->dac[index] = value;
};


void latero_set_DIO(latero_t* latero, uint16_t value) 
{
     latero->dio_out = value;
};
    

int latero_write_pins(latero_t* platero, double frame[LATERO_NB_PINS], latero_pkt_t* response)
{ 
	latero_set_pins(platero, frame);
    return latero_write(platero, response);
}


int latero_write_DIO(latero_t* platero, uint16_t value, latero_pkt_t* response)
{ 
	latero_set_DIO(platero, value);
    return latero_write(platero, response);
}

int latero_is_open(latero_t* latero)
{
    return latero->initialized;
}

// order of encoders is reversed
// direction is reversed
void latero_compute_position(latero_t *latero, uint32_t encoder_values[4], double *px, double *py, double *ptheta)
{
    double angles[3];
    angles[0] = -((int)encoder_values[2] - latero->encoder_offset[2]) * 2.0*M_PI / ENCODER_CPR;
    angles[1] = -((int)encoder_values[1] - latero->encoder_offset[1]) * 2.0*M_PI / ENCODER_CPR;
    angles[2] = -((int)encoder_values[0] - latero->encoder_offset[0]) * 2.0*M_PI / ENCODER_CPR;
    
    latero_forward_kinematics(angles, px, py, ptheta);
    *px = -*px + FULL_WORKSPACE_WIDTH - ROOT_OFFSET_X - DISC_DIAMETER/2; 
    *py = *py + ROOT_OFFSET_Y - DISC_DIAMETER/2;
    *ptheta = *ptheta - M_PI/2;
}


void latero_reset_position(latero_t *latero, uint32_t encoder_values[4])
{
    int encoder[3] = {encoder_values[0], encoder_values[1], encoder_values[2]};
    printf("[latero_reset_position] encoder values: %d %d %d\n", encoder[0], encoder[1], encoder[2]);
    
    // We're assuming that the Latero is in the lower-left corner of the
    // workspace, with the connector to the left. This means that the third
    // arm is oriented straight up. We can therefore use the inverse
    // kinematics of the 2-link problem (http://bit.ly/fZTaMY) to find the
    // first two joint angles. We can then deduce the orientation of the
    // third joint. 
    
    // reference position of second joint (mm, from root) [CHECKED]
    double refx = FULL_WORKSPACE_WIDTH - DISC_DIAMETER/2 - ROOT_OFFSET_X;
    double refy = FULL_WORKSPACE_HEIGHT - DISC_DIAMETER/2 - ROOT_OFFSET_Y - L2;
    printf("[latero_reset_position] Reference position of second joint (internal ref. frame): (%f,%f)\n", refx, refy);
    
    // compute first and second joint angle using inverse kinematics
    double a = L0;
    double b = L1;
    double x2 = refx*refx;
    double y2 = refy*refy;
    double x2py2 = x2+y2;
    double a2pb2 = a*a + b*b;
    double angles[3];
    angles[0] = atan2(refy,refx) - acos((a*a - b*b + x2py2)/(2*a*sqrt(x2py2)));
    angles[1] = acos((x2py2 - a2pb2)/(2*a*b));
    angles[2] = 0.5*M_PI - angles[0] - angles[1];
    printf("[latero_reset_position] Offset angles are: %f, %f and %f degrees.\n", angles[0]*180.0/M_PI, angles[1]*180.0/M_PI, angles[2]*180.0/M_PI); 
    
    // double-check with forward kinematics [CHECKED]
    //double x,y,theta;
    //latero_forward_kinematics(angles, &x, &y, &theta);
    //printf("forward kinematics: (%f, %f) %f\n", x, y, theta * 180.0 / M_PI);    
    
    // compute offset of encoders to zero degrees
    latero->encoder_offset[0] = encoder[0] + angles[2] * ENCODER_CPR / (2.0*M_PI); 
    latero->encoder_offset[1] = encoder[1] + angles[1] * ENCODER_CPR / (2.0*M_PI);
    latero->encoder_offset[2] = encoder[2] + angles[0] * ENCODER_CPR / (2.0*M_PI);
}