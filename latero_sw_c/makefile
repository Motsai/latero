DEPS = utils.c makefile
INC=-I../protocol

all: client

CC = gcc $(INC)

lat_client_api.o: lat_client_api.c lat_client_api.h

client: latero_client.c lat_client_api.o latero_testpattern.c $(DEPS)
	$(CC)  latero_testpattern.c latero_client.c utils.c lat_client_api.o -largtable2 -lm -o client

udptest: lateroUDPtest.c
	$(CC) lateroUDPtest.c -o udptest

clean:
	rm client udptest

