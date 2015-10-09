DEPS = utils.c makefile
INC=-I../protocol

REMOTE_USER = root

all: client server test_spi

CC = gcc $(INC)
CC_CROSS = arm-angstrom-linux-gnueabi-gcc

latero_spi.o: latero_spi.c $(DEPS)
	$(CC_CROSS) -c latero_spi.c -o latero_spi.o

test_spi : latero_spi.o test_spi.c
	$(CC_CROSS) -largtable2 test_spi.c latero_spi.o -o test_spi

server: latero_server.c latero_spi.o $(DEPS)
	$(CC_CROSS) latero_server.c utils.c latero_spi.o -o server

lat_client_api.o: lat_client_api.c lat_client_api.h

client: latero_client.c lat_client_api.o latero_testpattern.c $(DEPS)
	$(CC)  latero_testpattern.c latero_client.c utils.c lat_client_api.o -largtable2 -lm -o client

clean:
	rm client server *.o test_spi

# Remote operations (using ssh)
# Ensure that you have exported the LATERO_IP environment variable 
# before calling the commands below

latero_all : latero_install latero_test

# Performs a remote installation. 
latero_install: test_spi server
	echo "Copying the programs to the Latero"
	scp test_spi $(REMOTE_USER)@$(LATERO_IP):/home/root
	scp server $(REMOTE_USER)@$(LATERO_IP):/home/root

# Performs a remote test calling for the remote version to be displayed locally
latero_runserver: server
	echo "Remotely calling the server"
	ssh $(REMOTE_USER)@$(LATERO_IP) ./server


