CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic -I../src
SRC = ../src

vpath %.o $(SRC)

test: dvectest.c dvec.o list.o
	$(CC) $(CFLAGS) $^ -o dvectest

.PHONY: test
