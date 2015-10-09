#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#include <argtable2.h>

#include "latero.h"
#include "lat_client_api.h"

// #define TIMEOUTS_ENABLED


char print_level = 0;

void diep(char *s)
{
  perror(s);
  exit(1);
};



void clearQuadrature( unsigned char quad_index ) {
}



int my_main(const char* str_latero_ip, int print_response, int npack, 
            unsigned int* dacval, int dacval_cnt,
            char rd, char wr, int addr, int value, char dst_main, char dst_io, int testpattern )
{
  latero_conn latero;


  /*
  struct sockaddr_in si_server, si_other;
  int s, ii, slen=sizeof(si_other);
  char pktbuff[BUFLEN];
  char rspbuff[BUFLEN];
  */

  int ii;

  uint8_t  blades[64];
  uint16_t dio_out = 0x0000;
  // For raw address commands
  uint16_t saddr    = 0x0000;
  uint16_t sdata    = 0x0000;
  latero_dst_device destination;

  int numbytes = 0;

  latero_pkt_t response;

  printf("Init Connection: ");
  if ( ii = init_connection( &latero, str_latero_ip ) < 0 ) {
    printf("Error (%d)!\n",ii );
    return(-1);
  } else {
    printf("OK.\n");
  }

 
  // Sanitize the dac values
  for(ii = 0 ; ii < 4 ; ii++ ) {
      if (ii < dacval_cnt ) { 
          setDAC( &latero, ii, dacval[ii] );
      }
  }

/*	
  for(ii = 0 ; ii<64 ; ii++ ) {
      blades[ii] = ii;
  }
  setBlades( &latero, blades );
*/
  // Run a few test cases here
  TestInit();
  ii = 0;
  switch(testpattern) {	
    case 1:
        TestSplit1(&latero);
        sleep(1);
        TestSplit2(&latero);
    break;
    case 2:
        TestAllpin(&latero);
    break;
    case 3:
        TestRow(&latero);
        sleep(1);
        TestCol(&latero);
    break;
    case 4:
	// Test continuously without stopping
        while(1) {
		printf("%d ", ii);
		TestRow(&latero);
		printf("%d ", ii);
        	TestCol(&latero);
		ii++;
	}
    break;

  }
	
  /* Just on led is turned on... */
  setDIO( &latero, 0xDEAC );
  // setDAC( &latero, 0x0, 0x1234 );
  // setDAC( &latero, 0x1, 0x5678 );
  // setDAC( &latero, 0x2, 0x9ABC );
  // setDAC( &latero, 0x3, 0xDEF0 );

  printf("Sending\n");
  sendNormalPacket( &latero, &response );
  printf("printing\n");
  printPacket( &response );

/*
  printf("Test Connection\n");
  if( test_connection( &latero ) < 0 ) {
      fprintf(stderr,"The test_connection() failed\n");
  } else {
      printf("Connection with Latero is OK.\n");
  }
*/

  if( rd > 0 || wr > 0 ) {
    saddr = addr & 0xFFFF;
    sdata = value & 0xFFFF;

    if ( dst_main > 0 ) {
        destination = LATERO_CONTROLLER;
    } else if ( dst_io > 0 ) {
        destination = LATERO_IO;
    }
    if( rd > 0 ) {
        rawRead( &latero, destination, saddr, &sdata );
        printf("Data Read at address 0x%4.4X = 0x%4.4X\n", saddr,sdata);
    } else { /* Write */
        rawWrite(&latero, destination, saddr, sdata  );
        printf("0x%4.4X written at address 0x%4.4X\n", sdata,saddr );
    }
  } else {
    // Normal Latero Packets
    //fillNormalPacket(PKT_TYPE_FULL, dac, blades, dio_out, &packetSend);
  }

  return( close_connection( &latero ) );

}

int main(int argc, char **argv) {
    int exitcode = 0;
    int nerrors = 0;
    /* Prepare command line arguments */

    struct arg_str  *latero_ip = arg_str1(NULL,"latero_ip","IP", "Latero server IP address");    
    struct arg_lit  *print_resp= arg_lit0("p", "print",          "print response packet");
    struct arg_int  *numpkt    = arg_int0("n", "numpkt","<n>",   "How many packets (default is 1)");
    struct arg_int  *dacval    = arg_intn(NULL,"dac","<int>",0,4,"dac values (up to 4 values)");    
    struct arg_lit  *rd        = arg_lit0("r", "read",           "read");
    struct arg_lit  *wr        = arg_lit0("w", "write",          "write");
    struct arg_int  *addr      = arg_int0("a", "addr",NULL,      "address");
    struct arg_int  *value     = arg_int0("v", "value",NULL,     "value");
    struct arg_lit  *mainctrl  = arg_lit0(NULL,"mainctrl",       "Raw commands are for the main controller");

    struct arg_int  *tpat      = arg_int0("t","testpat","<n>",   "Run Test Pattern:1=Split, 2=AllPin, 3=RowCol");

    struct arg_lit  *latio     = arg_lit0(NULL,"lateroio",       "Raw commands are for the Latero IO card");
    struct arg_lit  *help      = arg_lit0(NULL,"help",           "print this help and exit");
    struct arg_end  *end       = arg_end(10);
    void*  argtable[] = {latero_ip,print_resp,numpkt,dacval,
                         rd, wr, addr, value, mainctrl, tpat, latio,
                         help,end};

    //latero_ip->sval[0] = "192.168.1.108";

    /* verify the argtable[] entries were allocated sucessfully */
    if (arg_nullcheck(argtable) != 0) {
        /* NULL entries were detected, some allocations must have failed */
        printf("Insufficient memory (argtable)\n");
        exitcode = 1; goto exit;
    }

    numpkt->ival[0] = 1;
    tpat->ival[0] = 0;

    nerrors = arg_parse(argc,argv,argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0) {
        printf("Usage: %s", argv[0]);
        arg_print_syntax(stdout,argtable,"\n");
        printf("Latero client demonstration program version 1, revision 1\n");
        arg_print_glossary(stdout,argtable,"  %-25s %s\n");
        exitcode=0; goto exit;
    }

    if (nerrors > 0) {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout,end,argv[0]);
        printf("Try '%s --help' for more information.\n",argv[0]);
        exitcode = 0; goto exit;
    }

    if( wr->count > 0 ) {
        if( addr->count != 1 || value->count != 1 ) {
            printf("Write requires an address and a value\n");
            exitcode = 1; goto exit;
        }
        if( mainctrl->count + latio->count == 0 ) {
            printf("Write requires a destination (--mainctrl or --lateroio)\n");
            exitcode = 1; goto exit;
        }
    }

    if( rd->count > 0 ) {
        if( addr->count != 1 ) {
            printf("Read requires an address\n");
            exitcode = 1; goto exit;
        }
        if( mainctrl->count + latio->count == 0 ) {
            printf("Read requires a destination (--mainctrl or --lateroio)\n");
            exitcode = 1; goto exit;
        }
    }
        

    return( my_main( latero_ip->sval[0], print_resp->count, numpkt->ival[0], dacval->ival, dacval->count,
                     rd->count, wr->count, addr->ival[0], value->ival[0], mainctrl->count, latio->count, tpat->ival[0]) );

    exit:
        printf("Program Ended\n");
        arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
        return(exitcode);

}

