CC = gcc 
CFLAGS = -O3 -Wall 
THREADFLAGS = -pthread 

all: 3_1 

3_1.o: 3_1.c 
	$(CC) $(CFLAGS) -c 3_1.c 

3_1: 3_1.o
	$(CC) 3_1.o -o 3_1 $(THREADFLAGS) $(CFLAGS)

clean:
	rm -f *.o 3_1 