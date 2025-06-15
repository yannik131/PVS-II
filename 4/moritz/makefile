CC = gcc 
CFLAGS = -O3 -Wall 
THREADFLAGS = -pthread 

all: 4_1 

4_1.o: 4_1.c 
	$(CC) $(CFLAGS) -c 4_1.c 

4_1: 4_1.o
	$(CC) 4_1.o -o 4_1 $(THREADFLAGS) $(CFLAGS)

run_4_1: 4_1
	./4_1

clean:
	rm -f *.o 4_1 4_2 4_3