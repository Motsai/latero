#Latero V2 Instruction Manual
###*For firmware version 2.2.5*
##Latero PC Software
The Latero V2 controller is compatible with only the motsai branch of the repository on [Jerome Pasquero's gitlab](https://gitlab.com/u/jerome.pasquero). This means that when you clone the software on your computer, you must checkout the motsai branch before compiling and running the software. Those files come from a public project derived from the Latero work at Tactile Labs.

The protocol files should be kept as-is and avoid any changes in the protocol to keep things compatible with other work.

It is however possible to extend the protocol.  In such case, a new packet type should be created and that packet type should be added the the latero_io.h file along with the unpack/pack methods. Changes to the protocol should be made public to help everyone.

##Using the Latero Controller
1. Plug the box into a 12V DC 1.5 Amp Power supply.
2. Press the power button to the ‘ON’ position.
3. At the time of startup, the STAT LED should turn orange. This means that the system is booting up. 
4. Once it is done booting up, the LED should turn either red if the head is disconnected from the box, or green if the head was connected.
5. If the tactile head was not connected, now is the time to connect it.
6. When the tactile head is connected, you can start the latero program on the PC. The IP address of the destio

##Indication LEDs
###STAT
The status LED gives out information about the state of the whole system. 
* **Orange (on startup)**: The system is booting up
* **Orange (during runtime):** An overcurrent event has occurred
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
Performing a system update is relatively straightforward. When the box is unpowered, connect a micro-USB cable from the PC to the prog USB port of the controller box. On the PC, a file explorer window title mbed will pop-up as if a flash drive had been connected. Drag and drop the firmware **laterov2_FW2-2-5_FPGA2-2-5.bin** *(for v2.2.5)* file into the window and wait until the file transfer has ended. When the mbed window pops up again it means the transfer is complete. You can disconnect the USB cable and restart the power on the 12V input jack.

##Overcurrent events
One of the main security features of the Latero controller is the current limiter. An overcurrent event occurs when the perceived current going through the high voltage circuit is a lot higher than expected (this would happen in the event of a short circuit or someone closing the circuit with their body). When this happens, the system automatically opens up the circuit feeding the high voltage booster, which immediately cuts off the high voltage power supply. The system can’t recover from this by itself, and you would need to perform an On-Off (5 second pause between the on and off) sequence with the back buttons.

##In case of a system crash, freeze
The Latero controller system is still under heavy development. It may still (occasionally) fail under certain conditions. When this happens, perform a system reboot, by putting the switch to ‘Off’, wait 5 seconds, and press ‘On’ again.
