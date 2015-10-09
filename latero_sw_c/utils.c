#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "latero.h"

void packPacket(char* pktbuff, unsigned int length, latero_pkt_t* pkt) 
{
	int ii, pos;
	uint16_t tmp;
	uint32_t tmpl;
	
	if (length < sizeof (latero_pkt_t))
		return;
	
    pktbuff[0] = pkt->hdr.magic;
    pktbuff[1] = pkt->hdr.version;
    pktbuff[2] = pkt->hdr.type;
	tmp =  htons(*(uint16_t*)&(pkt->hdr.seq));
	memcpy(&pktbuff[3], &tmp, sizeof(uint16_t));
	
	pos = 5;
	switch (pkt->hdr.type) {
		case PKT_TYPE_FULL: case PKT_TYPE_IO:
            tmp =  htons(*(uint16_t*)&(pkt->full.dio_out));
            memcpy(&pktbuff[pos], &tmp, sizeof(uint16_t));pos+=2;

			//pktbuff[pos] = pkt->full.dio_out; pos+=2;
			
			for( ii = 0 ; ii < 4 ; ii++ ) {
				tmp = htons(*(uint16_t*)&(pkt->full.dac[ii])); 
				memcpy(&pktbuff[pos+2*ii], &tmp, sizeof(uint16_t) ); 
			} 
			pos = pos+2*ii;
			for( ii = 0 ; ii < 64 ; ii++ ) {
				pktbuff[pos+ii] = pkt->full.blade[ii]; 
			}
			break;
		case PKT_TYPE_FULLR0: case PKT_TYPE_FULLR1:
            tmp =  htons(*(uint16_t*)&(pkt->fullr.dio_in));
            memcpy(&pktbuff[pos], &tmp, sizeof(uint16_t));pos+=2;
			//pktbuff[pos] = pkt->fullr.dio_in; pos+=2;

			tmp =  htons(*(uint16_t*)&(pkt->fullr.ctrlstatus));
			memcpy(&pktbuff[pos], &tmp, sizeof(uint16_t));pos+=2;
			tmp =  htons(*(uint16_t*)&(pkt->fullr.iostatus));
			memcpy(&pktbuff[pos], &tmp, sizeof(uint16_t));pos+=2;
			
			for( ii = 0 ; ii < 4 ; ii++ ) {
				tmpl = htonl(*(uint32_t*)&(pkt->fullr.quad[ii])); 
				memcpy(&pktbuff[pos+sizeof(uint32_t)*ii], &tmpl, sizeof(uint32_t) ); 
			}
			pos = pos+sizeof(uint32_t)*ii;
			for( ii = 0 ; ii < 4 ; ii++ ) {
				tmp = htons(*(uint16_t*)&(pkt->fullr.adc[ii])); 
				memcpy(&pktbuff[pos+sizeof(uint16_t)*ii], &tmp, sizeof(uint16_t) ); 
			}
			break;
		case PKT_TYPE_RAW:	case PKT_TYPE_RAWR:
			pktbuff[pos] = pkt->raw.command; pos++;
			tmp =  htons(*(uint16_t*)&(pkt->raw.address));
			memcpy(&pktbuff[pos], &tmp, sizeof(uint16_t));pos+=2;
			tmp =  htons(*(uint16_t*)&(pkt->raw.data));
			memcpy(&pktbuff[pos], &tmp, sizeof(uint16_t));pos+=2;
			break;
	}
}


int unpackPacket(char* buf, unsigned int length, latero_pkt_t* pkt) 
{
	int ii, pos;
	uint16_t tmp;
	uint32_t tmpl;
	
    /* TODO Make this test more robust and based on actual received packet format
            expected length. */
	if (length < sizeof (latero_pkt_t)) { return(-1); }
	
	pkt->hdr.magic      = buf[0];
	pkt->hdr.version    = buf[1];
	pkt->hdr.type       = buf[2];
	pkt->hdr.seq        = ntohs(*(uint16_t*)(&buf[3]));
	pos = 5;

    /* Some basic sanity in case we receive another type of UDP packet */
    /* Avoid crash and burn... */
    if ( pkt->hdr.magic != LATERO_MAGIC_NB ) {
        return(-1);
    }
	
	switch (pkt->hdr.type) {
			
		case PKT_TYPE_FULL: 
        case PKT_TYPE_IO:
			/* printf("Packet Type Full or IO\n"); */
			pkt->full.dio_out = buf[pos]; pos+=2;
			for( ii = 0 ; ii < 4 ; ii++ ) {
				pkt->full.dac[ii] = ntohs(*(uint16_t*)(&buf[pos+2*ii]));
			}
			pos = pos+2*ii;		
			if (pkt->hdr.type == PKT_TYPE_FULL) {
				for( ii = 0 ; ii < 64 ; ii++ ) {
					pkt->full.blade[ii] = buf[pos+ii];
				}
			} 
			break;
			
		case PKT_TYPE_RAW: 
        case PKT_TYPE_RAWR:
			/* printf("Packet Type Raw \n"); */
			pkt->raw.command = buf[pos]; pos++;
			pkt->raw.address = ntohs(*(uint16_t*)(&buf[pos])); pos+=2;
			pkt->raw.data    = ntohs(*(uint16_t*)(&buf[pos])); pos+=2;
			break;
			
		case PKT_TYPE_FULLR0: 
        case PKT_TYPE_FULLR1:
			/*printf("Packet Type Full Response \n");*/
			
			pkt->fullr.dio_in = ntohs(*(uint16_t*)(&buf[pos])); pos+=2;
			pkt->fullr.ctrlstatus = ntohs(*(uint16_t*)(&buf[pos])); pos+=2;
			pkt->fullr.iostatus   = ntohs(*(uint16_t*)(&buf[pos])); pos+=2;
			
			for( ii = 0 ; ii < 4 ; ii++ ) {
				pkt->fullr.quad[ii] = ntohl(*(uint32_t*)(&buf[pos+sizeof(uint32_t)*ii])); 
			}
			pos = pos+sizeof(uint32_t)*ii;
			for( ii = 0 ; ii < 4 ; ii++ ) {
				pkt->fullr.adc[ii] = ntohs(*(uint16_t*)(&buf[pos+sizeof(uint16_t)*ii])); 
			}
			break;
        default:
            printf("Unknown packet type!!!\n");
            return(-2);			
	}	
    return(0);
}

void printPacket(latero_pkt_t* packet) {
	
	int ii;
    printf("pkt.magic=0x%2.2X (%s)\n", packet->hdr.magic, packet->hdr.magic==LATERO_MAGIC_NB?"Valid":"Invalid");
    printf("pkt.version=0x%2.2X\n", packet->hdr.version);
    printf("pkt.type=0x%2.2X\n", packet->hdr.type);
    printf("pkt.seq=0x%4.4X\n", packet->hdr.seq);
	
	switch (packet->hdr.type) {
			
		case (PKT_TYPE_FULL): case (PKT_TYPE_IO):
			printf("pkt.dio_out=0x%2.2X\n", packet->full.dio_out);
			for( ii = 0 ; ii < 4 ; ii++ ) {
				printf("pkt->full.dac[%d]=0x%4.4X\n", ii, packet->full.dac[ii] );
			}
			
			if (packet->hdr.type == PKT_TYPE_FULL) {
				printf("pkt->full.blade=[");
				for( ii = 0 ; ii < 64 ; ii++ ) {
					printf("0x%2.2X, ", packet->full.blade[ii]);
				}
				printf("]\n");
			} 
			break;
			
		case (PKT_TYPE_RAW): case PKT_TYPE_RAWR:
			printf("pkt->raw.command=0x%2.2X\n", packet->raw.command);
			printf("pkt->raw.address=0x%4.4X\n", packet->raw.address);
			printf("pkt->raw.data=0x%4.4X\n", packet->raw.data);
			break;
			
		case PKT_TYPE_FULLR0: case PKT_TYPE_FULLR1:
			printf("pkt->fullr.dio_in=0x%4.4X\n", packet->fullr.dio_in);
			printf("pkt->fullr.ctrlstatus=0x%4.4X\n", packet->fullr.ctrlstatus);
            printf("  Controller Details\n");
            printf("  FPGA Version %d, Rev %d\n", 
                   (packet->fullr.ctrlstatus & 0x0F00) >> 8,
                   (packet->fullr.ctrlstatus & 0x00F0) >> 4 );
            printf("  High Voltage State : %s%s\n", 
                     (packet->fullr.ctrlstatus & 0x0002) > 0 ? "POWERED_OFF" : "",
                     (packet->fullr.ctrlstatus & 0x0004) > 0 ? "ACTIVE" : "");
            printf("  Active Page : %d\n",
                     (packet->fullr.ctrlstatus & 0x0001));

			printf("pkt->fullr.iostatus=0x%4.4X\n", packet->fullr.iostatus);

            if ( packet->fullr.iostatus == 0x0000 ) {
                printf("  Warning !! The LateroIO status is invalid.\n\
  The Latero I/O interface\n\
  is likely to be unplugged or not powered on.\n");
            } else {
                printf("  LateroIO Details\n");
                printf("  FPGA Version %d, Rev %d\n", 
                          (packet->fullr.iostatus & 0x0F00) >> 8,
                          (packet->fullr.iostatus & 0x00F0) >> 4 );
                printf("  Quadrature Decoder Error Mask = %1.1X\n",
                          (packet->fullr.iostatus & 0x000F) );

			    for(ii = 0 ; ii < 4 ; ii++ ) {
				    printf("pkt->fullr.quad[%d]=0x%8.8X (%d)\n", ii, 
                            packet->fullr.quad[ii], packet->fullr.quad[ii] );
			    }
			    for(ii = 0 ; ii < 4 ; ii++ ) {
				    printf("pkt->fullr.adc[%d]=0x%4.4X (%2.3f Volts)\n", ii, 
                            packet->fullr.adc[ii],
                            (int16_t)packet->fullr.adc[ii] * 10.0  / 32768.0);
			    }
            }
			break;
			
    }
}

