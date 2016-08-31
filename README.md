#What is a Latero ?
The Latero is the name that was given from a project done under the supervision of Prof. Vincent Hayward at McGill University in Montreal.  The aim of this work was to develop the illusion of little tactile dots pushing against the finger pad, but only using lateral skin deformation and leveraging the way the brain interpret the skin deformation.

[The original work behind the Laterotactile controller](http://www.cim.mcgill.ca/~haptic/laterotactile/index.php)

The original devices required a rather bulky setup of high-voltage power supply to drive the piezoelectric bimorph benders along with a PC with rather complex real-time control cards.  The team at Motsai has worked over the years to miniaturize the technology to a point where the complete system can be easily driven with a laptop and only requires a small control box that includes all the critical components to drive the piezoelectric bender array.

The controller leverages FPGA logic to ensure accurate real-time refresh of the tactile blades and proper powering sequences to the high-voltage amplifiers.  The Latero Controller has been augmented in this 2nd overhaul with a complete acquisition system in a way that can be used to time correlate force sensors, quadrature encoders and output real-time voltage that is correlated with the motion of the tactile blades.  While not exactly designed for home use (its really a research instruments), we feel that there is certainly many uses of the technology that could be used to improve the lives of some visually impaired individuals (Braille displays) for example.

Motsai does not distribute or sell this unit directly.  To get more information, interested parties should contact [Tactile Labs](http://www.tactilelabs.com)

Motsai would be happy to discuss with potential customers interested in real-time control of piezoelectric elements (up to 64 simultaneous channels at 200V).  There are few systems on the market that can drive so many channels economically. 

#Latero V2 Instructions
###*For firmware version 2.3.4*

![Latero whole setup](http://i.imgur.com/I7360KQ.jpg)

<!-- ##Getting started with the Latero -->
##Compiling the PC software

There are build tools required to be able to build and run the Latero software. You should have:
+ POSIX-compatible operating system (OS X Or Linux)
+ GCC and its associated C libraries
+ libargtable2 (if you don't have this don't worry, it is present in this repo)

Start by cloning the git repository onto your computer:
```
git clone https://github.com/Motsai/latero.git
```

If you do not have the argtable2 library installed on your computer, you can install the one that is included in this repository. 
```
cd latero/latero_sw_c
tar -xvf argtable2-13.tar
cd argtable2-13
./configure
sudo make install
make clean
```
Alternatively, Ubuntu users can simply do 
```
sudo apt-get install libargtable2-dev
```

Next, go to the directory with the code and makefile and run the makefile to build the client.
```
cd ..
make client
```
Assuming you have all the necessary libraries, the output of the console should look a little bit like this:
```
gcc -I../protocol    -c -o lat_client_api.o lat_client_api.c
gcc -I../protocol  latero_testpattern.c latero_client.c utils.c lat_client_api.o -largtable2 -lm -o client
```

**Note**: The software has not been tested to work with Windows. It might be possible to hack it together using cygwin.

##Starting up the Latero Controller box
1. Plug the box into a 12V DC 1.5 Amp Power supply.
2. Press the power button to the 'ON' position.
3. At the time of startup, the STAT LED should turn orange. This means that the system is booting up. 
4. Once it is done booting up, the LED should turn either red if the head is disconnected from the box, or green if the head was connected.
5. If the tactile head was not connected, now is the time to connect it.
6. When the tactile head is connected, you can start the latero program on the PC.

##Communicating with the Latero

###Running the application
Make sure the Latero controller is up and running according to instructions and connected to the PC by an ethernet cable. Now, you can run the PC software with a given test pattern:
```
./client --latero_ip=192.168.87.98 -t 1
```

![Latero ethernet](http://i.imgur.com/Q8nIJVd.jpg)

###Communication through Ethernet
Communication with the Latero controller is done through an ethernet interface using UDP packets. It is recommended that the ethernet connection be directly from the PC to the controller, as opposed to routing it through a network. Communicating with the controller through your local LAN may work, but it will negatively affect the response time by adding either jitter and/or latency.

###Protocol files
The protocol files should be kept as-is to avoid breaking communication between the controller and the PC. Any changes made by Motsai in the protocol from the firmware side will be documented and updated accordingly.

##Indication LEDs
![Latero front](http://i.imgur.com/5UgTBS7.jpg)
###STAT
The status LED gives out information about the state of the whole system. 
* **Orange (on startup)**: The system is booting up
* **Orange (during runtime)**: An overcurrent event has occurred
* **Red**: Either the tactile head cable is not connected or a fatal error has occurred.
* **Green**: The controller is ready to accept new input 

###NET
The NET led turns on and off for each individual packet received. When a stream of packets is being transmitted to the Latero, the LED will toggle for each one, which will basically make it appear dimmed. If the NET LED is is off, it means that the firmware has probably crashed.

###HV
The HV (High Voltage) LED represents the state of the 200V and 100V supply lines that go to the tactile head.
* **Red**: The high voltage power lines are live. Be very cautious while touching the tactile head when it is in this state.
* **Green**: The high voltage power lines are inactive. Sending packets to the controller will re-activate the high voltage.

###EXP
The EXP LED blinking indicates activity on the expansion connector. This LED is currently green by default in this firmware version.

##Performing a system update
Performing a system update is relatively straightforward. When the box is unpowered, connect a micro-USB cable from the PC to the prog USB port of the controller box. On the PC, a file explorer window title mbed will pop-up as if a flash drive had been connected. Drag and drop the firmware [latest here](https://github.com/Motsai/latero/tree/master/firmware_bin) file into the window and wait until the file transfer has ended. When the mbed window pops up again it means the transfer is complete. You can disconnect the USB cable and restart the power on the 12V input jack.

![SystemUpdate](http://i.imgur.com/TyVFInp.png)

##Overcurrent events
One of the main security features of the Latero controller is the current limiter. An overcurrent event occurs when the perceived current going through the high voltage circuit is a lot higher than expected (this would happen in the event of a short circuit or someone closing the circuit with their body). When this happens, the system automatically opens up the circuit feeding the high voltage booster, which immediately cuts off the high voltage power supply. The system can’t recover from this by itself, and you would need to perform an On-Off (5 second pause between the on and off) sequence with the back buttons.

##In case of a system crash, freeze
The Latero controller system is still under heavy development. It may still (occasionally) fail under certain conditions. When this happens, perform a system reboot, by putting the switch to ‘Off’, wait 5 seconds, and press ‘On’ again.

##Latero Mac OS X GUI Software
Some developers may want to develop applications based on the Latero GUI software developed for an OS X runtime. This particular software is available on [Jerome Pasquero's gitlab](https://gitlab.com/u/jerome.pasquero). The Latero V2 controller is compatible with only the motsai branch of that repository. This means that when you clone the software on your computer, you must checkout the motsai branch before compiling and running the software. Those files come from a public project derived from the Latero work at Tactile Labs.

**Note**: The gitlab repository is not officially maintained by Motsai. If you have requests for additional features or issues with the software, it should be addressed to the official maintainers of that software.

