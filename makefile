#
# Makefile for simple test of concurrent buffer
#

CC = gcc
CFLAGS = -g -Wall -std=c99 -pthread
EXES = histogram

all: $(EXES)

histogram: histogram.c concurrentBuffer.o
	$(CC) $(CFLAGS) histogram.c concurrentBuffer.o -o histogram

concurrentBuffer.o: concurrentBuffer.c
	$(CC) $(CFLAGS) -c concurrentBuffer.c

clean:
	-rm -f histogram.o $(EXES)
