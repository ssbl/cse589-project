CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic

dvec: dvec.c
	$(CC) $(CFLAGS) $^ -c

table: table.c
	$(CC) $(CFLAGS) $^ -c

serventry: serventry.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c

.PHONY: dvec table
