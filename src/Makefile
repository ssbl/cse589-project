CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic

all: list dvec table serventry topo pack unpack

list: list.c
	$(CC) $(CFLAGS) $^ -c

dvec: dvec.c
	$(CC) $(CFLAGS) $^ -c

table: table.c
	$(CC) $(CFLAGS) $^ -c

serventry: serventry.c
	$(CC) $(CFLAGS) $^ -c

topo: topo.c
	$(CC) $(CFLAGS) $^ -c

pack: pack.c
	$(CC) $(CFLAGS) $^ -c

unpack: unpack.c
	$(CC) $(CFLAGS) $^ -c

%.o: %.c
	$(CC) $(CFLAGS) $< -c

.PHONY: list dvec table serventry topo pack unpack
