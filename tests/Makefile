# run make in src first

CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic -I../src
SRC = ../src

vpath %.o $(SRC)

dvec: dvectest.c dvec.o list.o
	$(CC) $(CFLAGS) $^ -o dvectest

table: tabletest.c table.o dvec.o list.o
	$(CC) $(CFLAGS) $^ -o tabletest

.PHONY: dvec table
