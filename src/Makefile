CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic

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

msg: msg.c list.o dvec.o table.o serventry.o topo.o
	$(CC) $(CFLAGS) $^ -o msg

%.o: %.c
	$(CC) $(CFLAGS) $< -c

.PHONY: list dvec table serventry topo msg
