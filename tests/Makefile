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

.PHONY: dvec table serventry topo
