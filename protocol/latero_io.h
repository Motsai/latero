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

#ifndef LATERO_IO_H 
#define LATERO_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PORT 8900
#define LATERO_MAGIC_NB 0xCA

/* Latero packet types */
/* Request Packets*/
#define PKT_TYPE_FULL 0x01
#define PKT_TYPE_IO   0x02
#define PKT_TYPE_RAW  0x03

/* Response Packets*/
#define PKT_TYPE_FULLR0 0x81
#define PKT_TYPE_FULLR1 0x82
#define PKT_TYPE_RAWR   0x83

/* Version 1, Revision 1 of the protocol */
#define PKT_VER_REV     0x11


#define PKT_RAW_CMD_RD   0x01
#define PKT_RAW_CMD_WR   0x02
#define PKT_RAW_CMD_CTRL 0x10
#define PKT_RAW_CMD_IO   0x20

#define BUFLEN 100
/* #define UDP_ADDR "127.0.0.1" */

#define LATERO_EXPAN_DIO       0x10 /* Read = DIN, Write = DOUT */
#define LATERO_EXPAN_DIO_DIR   0x11 /* 0 = input, 1 = output */


typedef struct {
    uint8_t  magic;     // Magic number 0xCA
    uint8_t  version;   // Version(4bits):Revision(4bits)
    uint8_t  type;      // Packet Types	
	uint16_t seq;		// Sequence Number
} latero_hdr_t;

typedef struct {
	latero_hdr_t hdr; // 5 bytes

	union {

		struct {
			// fields for full frame PKT_TYPE_FULL
			uint16_t dio_out;	// Digital IO outputs 
			uint16_t dac[4];    // DAC outputs 8 bytes
			uint8_t  blade[64]; // Blade positions (only present in normal packets)
		} full;
		
		struct {
			// fields for full frame rsp PKT_TYPE_FRSP0 and PKT_TYPE_FRSP1
			uint16_t dio_in;		  // Digital IO inputs
			uint16_t ctrlstatus;      // controller status
			uint16_t iostatus;        // io status
			uint32_t quad[4];         // quadrature encoders
			uint16_t adc[4];          // ADC Inputs
		} fullr;
			
		struct {
			// fields for info response PKT_TYPE_IRSP
			uint8_t command;
			uint16_t address;
			uint16_t data;	
		} raw;
	};
	
} latero_pkt_t;


void packPacket(char* pktbuff, unsigned int length, latero_pkt_t* pkt);

int unpackPacket(char* buf, unsigned int length, latero_pkt_t* pkt);


#ifdef __cplusplus
}
#endif

#endif //LATERO_H


