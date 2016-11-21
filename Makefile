CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic
MAKE = make server

all:
	cd src && $(MAKE) && cp server ..
