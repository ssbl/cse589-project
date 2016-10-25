CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic

dvec: dvec.c
	$(CC) $(CFLAGS) $^ -c

table: table.c dvec.o list.o
	$(CC) $(CFLAGS) $^ -o $@

serventry: serventry.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c

.PHONY: dvec table
