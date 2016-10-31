# run make in src first

CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic -I../src
SRC = ../src

vpath %.o $(SRC)

dvec: dvectest.c dvec.o list.o
	$(CC) $(CFLAGS) $^ -o dvectest

table: tabletest.c table.o dvec.o list.o
	$(CC) $(CFLAGS) $^ -o tabletest

serventry: serventrytest.c serventry.o
	$(CC) $(CFLAGS) $^ -o serventrytest

topo: topotest.c topo.o list.o dvec.o serventry.o table.o
	$(CC) $(CFLAGS) $^ -o topotest

pack: packtest.c pack.o list.o dvec.o table.o serventry.o table.o topo.o
	$(CC) $(CFLAGS) $^ -o packtest

unpack: unpacktest.c unpack.o pack.o list.o dvec.o table.o \
		serventry.o table.o topo.o
	$(CC) $(CFLAGS) $^ -o unpacktest

.PHONY: dvec table serventry topo pack
