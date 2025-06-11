CC = gcc
CFLAGS = -Wall -O3
THREADFLAGS = -pthread

all: 4_1

# Build the common object files.
util.o: util.c
	$(CC) $(CFLAGS) -c util.c

vector.o: vector.c
	$(CC) $(CFLAGS) -c vector.c

4_1.o: 4_1.c
	$(CC) $(CFLAGS) -c 4_1.c

# Link the executables.
4_1: util.o vector.o 4_1.o
	$(CC) util.o vector.o 4_1.o -o 4_1 $(CFLAGS)

clean:
	rm -f *.o 4_1