# Send a packet periodically to the latero using netcat
# To build udptest, do the command "make udptest"
./udptest 10000 | nc -u  -p 51864 192.168.87.98 8900
