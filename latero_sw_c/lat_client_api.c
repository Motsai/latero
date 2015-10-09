
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include "latero.h"
#include "lat_client_api.h"

// #define TIMEOUTS_ENABLED

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

int exchange_packet( latero_conn* latero, latero_pkt_t* to_send, latero_pkt_t* response ) {
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

int sendNormalPacket( latero_conn* latero, latero_pkt_t* response ) {
	int ii;
    latero_pkt_t pkt;

	pkt.hdr.magic   = LATERO_MAGIC_NB;
	pkt.hdr.version = PKT_VER_REV; 
    pkt.hdr.type    = PKT_TYPE_FULL;
	pkt.full.dio_out = latero->dio_out;
    for(ii = 0 ; ii < 4 ; ii++ ) {
      pkt.full.dac[ii] = latero->dac[ii];
    }
    for (ii = 0; ii<64; ii++) {
	  pkt.full.blade[ii] = latero->blades[ii];
	}
    return( exchange_packet( latero, &pkt, response ) );
}

int rawWrite( latero_conn* latero, latero_dst_device destination, 
              uint16_t address, uint16_t data ) {
  uint16_t command;
  latero_pkt_t response;
  latero_pkt_t request;
  int retcode;

  command = PKT_RAW_CMD_WR;
  if ( destination == LATERO_CONTROLLER ) {
    command += PKT_RAW_CMD_CTRL;
  } else if ( destination == LATERO_IO ) {
    command += PKT_RAW_CMD_IO;
  }
  /* Prepare raw command request packet */
  raw_cmd_packet( command, address, data, &request);
  retcode = exchange_packet( latero, &request, &response );
  return(retcode);
}

int rawRead( latero_conn* latero, latero_dst_device destination, 
             uint16_t address, uint16_t* data_read ) {
  uint16_t command;
  latero_pkt_t response;
  latero_pkt_t request;
  int retcode;
  command = PKT_RAW_CMD_RD;
  if ( destination == LATERO_CONTROLLER ) {
    command += PKT_RAW_CMD_CTRL;
  } else if ( destination == LATERO_IO ) {
    command += PKT_RAW_CMD_IO;
  }
  /* Prepare raw command request packet */
  raw_cmd_packet( command, address, 0x0000, &request);
  retcode = exchange_packet( latero, &request, &response );
  /* Extract the returned data (data remotely read) */
  *data_read = response.raw.data;
  return(retcode);
}

int init_connection( latero_conn* latero, const char* str_ip_address ) {
  int ii;

  latero->initialized = 0;

  memset( (char *)&latero->si_server, 0, sizeof(struct sockaddr_in));
  latero->si_server.sin_family = AF_INET;
  latero->si_server.sin_port = htons(PORT);
  latero->si_server.sin_addr.s_addr = inet_addr(str_ip_address);

  memset( (char *)latero->pktbuff, 0xFF, BUFLEN );
  memset( (char *)latero->rspbuff, 0xFF, BUFLEN );
  
  for( ii = 0 ; ii < 4 ; ii++ ) {
    latero->dac[ii] = 0;
  }

  for( ii = 0 ; ii < 64 ; ii++ ) {
    latero->blades[ii] = 0x80; /* Initialize to mid position */
  }

  latero->dio_out = 0x1000; /* Now only one LED will be on */

  latero->udp_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
  if ( latero->udp_socket == -1 ) {
    return(-1);
  }
  
  latero->initialized = 1;
  return(0);

}

int test_connection( latero_conn* latero ) {
  latero_pkt_t response;
  sendNormalPacket( latero, &response );
  printf("Response from Latero:\n");
  printPacket(&response);
}

int close_connection( latero_conn* latero ) {
    if ( close( latero->udp_socket ) != 0  ) {
      fprintf(stderr,"Closing failed on socket\n");
      return(-1);
    }
    return( 0 );
}

void setBlades( latero_conn* latero, char* blade_values ) {
    int ii;
    for( ii = 0 ; ii < 64 ; ii++ ) {
        latero->blades[ii] = blade_values[ii];
    }
};

void setDAC( latero_conn* latero, char index, uint16_t value ) {
    assert( index < 4 && index >= 0 );
    latero->dac[index] = value;
};

void setDIO( latero_conn* latero, uint16_t value ) {
    latero->dio_out = value;
};


/* The command below are helper commands */

int writeDIODir(latero_conn* latero, uint16_t value ) {
    return( rawWrite( latero, LATERO_IO, 0x18, value ) );
}

int readDIODir( latero_conn* latero, uint16_t *value) {
    return( rawRead( latero, LATERO_IO, 0x18, value ) );
}


