Source code for the Latero client/server UDP protocol and SPI interface.

Notes:
The server runs on the ARM-based controller of the Latero.  Therefore it must be 
cross-compiled.

The client can either run on the ARM-based controller (loopback interface) or on a
remote station natively compiled.

**********
Building the client (Linux or Mac OS X on the command line)
**********

1- Uncompress the argtable library archive.  
$ tar xzvf argtable2-13.tar
$ cd argtable2-13
$ ./configure
$ make
$ sudo make install
$ cd ..

Alternatively, in Ubuntu (10.04 LTS), you can install the libargtable2-dev package:
$ sudo apt-get install libargtable2-dev

2- Build the client
$ make client

************
Client Usage
************

1- Plug the Latero controller in a network switch.
2- Start the Latero controller and wait for about 30 seconds for it to boot up and show activity on the LAN leds.  The IP address of the Latero controller is fixed and the computer communicating with the Latero controller should be on the same domain (192.168.1.x).

The simplest way to use the client demonstration code is to call it from the command line.
$ ./client --help
Latero client demonstration program version 1, revision 1
  --latero_ip=IP            Latero server IP address
  -p, --print               print response packet
  -n, --numpkt=<n>          How many packets (default is 1)
  --dac=<int>               dac values (up to 4 values)
  -r, --read                read
  -w, --write               write
  -a, --addr=<int>          address
  -v, --value=<int>         value
  --mainctrl                Raw commands are for the main controller
  --lateroio                Raw commands are for the Latero IO card
  --help                    print this help and exit
Program Ended

The next step is to attempt to connect with the Latero server at the address (in our setup, the ip address is .116, but you can read this information on the Latero box):
$ ./client --latero_ip=192.168.1.116
Init Connection: OK.
Sending
printing
pkt.magic=0xCA (Valid)
pkt.version=0x11
pkt.type=0x81
pkt.seq=0x74E0
pkt->fullr.dio_in=0x007F
pkt->fullr.ctrlstatus=0x7124
  Controller Details
  FPGA Version 1, Rev 2
  High Voltage State : ACTIVE
  Active Page : 0
pkt->fullr.iostatus=0x715F
  LateroIO Details
  FPGA Version 1, Rev 5
  Quadrature Decoder Error Mask = F
pkt->fullr.quad[0]=0xFFFFFD73 (-653)
pkt->fullr.quad[1]=0x0000069A (1690)
pkt->fullr.quad[2]=0x00000184 (388)
pkt->fullr.quad[3]=0x000002AA (682)
pkt->fullr.adc[0]=0x003A (0.003625 Volts)
pkt->fullr.adc[1]=0x000D (0.000813 Volts)
pkt->fullr.adc[2]=0x000E (0.000875 Volts)
pkt->fullr.adc[3]=0x0015 (0.001313 Volts)

*****
Notes
*****
It is possible that the first connection attempt results in a timeout with an error message like this:
Init Connection: OK.
Sending
Warning! Timeout waiting from response from the Latero
  - Ensure that you use the right IP to communicate with the server.
  - Check that the server is running on the Latero.
printing
pkt.magic=0xFF (Invalid)
pkt.version=0xFF
pkt.type=0xFF
pkt.seq=0xFFFF
pkt->fullr.dio_in=0x007F
pkt->fullr.ctrlstatus=0x7124
  Controller Details
  FPGA Version 1, Rev 2
  High Voltage State : ACTIVE
  Active Page : 0
pkt->fullr.iostatus=0x715F
  LateroIO Details
  FPGA Version 1, Rev 5
  Quadrature Decoder Error Mask = F
pkt->fullr.quad[0]=0xFFFFFD73 (-653)
pkt->fullr.quad[1]=0x0000069A (1690)
pkt->fullr.quad[2]=0x00000184 (388)
pkt->fullr.quad[3]=0x000002AA (682)
pkt->fullr.adc[0]=0x003A (0.003625 Volts)
pkt->fullr.adc[1]=0x000D (0.000813 Volts)
pkt->fullr.adc[2]=0x000E (0.000875 Volts)
pkt->fullr.adc[3]=0x0015 (0.001313 Volts)


This can happen if the latero controller has been idle for a long time and the controller went in low-power mode.  A second connection attempt will not result in a timeout.  The timeouts are very short (you can change that in the source code).  The network connection should ideally be direct from the computer to the latero using a cross Ethernet cable or via a single switch that does not add latency to the network.

