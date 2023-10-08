# Makefile for building both server and client programs

CC = gcc
CFLAGS = -Wall

all: 

	@echo "Building the server..."
	$(CC) $(CFLAGS) -o mathserver/server mathserver/src/server231.c


	@echo "Building the client..."
	$(CC) $(CFLAGS) -o client/client231 client/src/client231.c

clean:
	rm -f server client231

