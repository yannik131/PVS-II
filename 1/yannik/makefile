CC = gcc
CFLAGS = -Wall -O3
THREADFLAGS = -pthread

all: 1_1 1_2 1_3

# Build the common object files.
util.o: util.c
	$(CC) $(CFLAGS) -c util.c

vector.o: vector.c
	$(CC) $(CFLAGS) -c vector.c

1_1.o: 1_1.c
	$(CC) $(CFLAGS) -c 1_1.c

1_2.o: 1_2.c
	$(CC) $(CFLAGS) -c 1_2.c

1_3.o: 1_3.c
	$(CC) $(CFLAGS) -c 1_3.c

# Link the executables.
1_1: util.o vector.o 1_1.o
	$(CC) util.o vector.o 1_1.o -o 1_1 $(CFLAGS)

1_2: util.o vector.o 1_2.o
	$(CC) util.o vector.o 1_2.o -o 1_2 $(THREADFLAGS) $(CFLAGS)

1_3: util.o vector.o 1_3.o
	$(CC) util.o vector.o 1_3.o -o 1_3 $(CFLAGS)

clean:
	rm -f *.o 1_1 1_2 1_3