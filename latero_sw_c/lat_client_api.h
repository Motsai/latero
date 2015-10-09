/*
Latero Client API

(C) 2010 Tactile Labs Inc.

Author : Jean-Samuel Chenard (chenard@tactilelabs.com)

*/

#ifndef __LAT_CLIENT_API
#define __LAT_CLIENT_API
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "latero.h"


/* Opaque structure that defines a connection with the server 
   Maintains a set of states and buffers.  Elements of this structure
   should only be modified by this API, not directly by the client.
*/
typedef struct lat_srv_con {
  char   initialized;

  int    udp_socket;   

  struct sockaddr_in si_server;
  
  char   pktbuff[BUFLEN];
  char   rspbuff[BUFLEN];

  uint16_t dac[4];
  uint8_t  blades[64];
  uint16_t dio_out;
} latero_conn;

//typedef enum x { LATERO_CONTROLLER, LATERO_IO } dst_device;

/* Will attempt a connection with the server and see if it can talk with it 
   returns 0 if successful (negative numbers if not).
*/
int init_connection( latero_conn* latero, const char* str_ip_address );

int test_connection( latero_conn* latero );

int close_connection( latero_conn* latero );

void setBlades( latero_conn* latero, char* blade_values );

void setDAC( latero_conn* latero, char index, uint16_t value );

void setDIO( latero_conn* latero, uint16_t value );

int sendNormalPacket( latero_conn* latero, latero_pkt_t* response );

int rawWrite( latero_conn* latero, latero_dst_device destination, 
              uint16_t address, uint16_t data );

int rawRead( latero_conn* latero, latero_dst_device stination, 
             uint16_t address, uint16_t* data_read );

#endif

