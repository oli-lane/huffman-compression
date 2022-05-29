# Makefile for huffman encode and decode

CC = gcc
CFLAGS = -Wall -ansi -pedantic -g

all: hencode hdecode

hencode: hencode.c huffstuff.c
	$(CC) $(CFLAGS) -o hencode hencode.c huffstuff.c

hdecode: hdecode.c huffstuff.c
	$(CC) $(CFLAGS) -o hdecode hdecode.c huffstuff.c

htable: htable.c huffstuff.c
	$(CC) $(CFLAGS) -o htable htable.c huffstuff.c

clean:
	rm -f *.o
