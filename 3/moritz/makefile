CC = gcc 
CFLAGS = -O3 -Wall 
THREADFLAGS = -pthread 

all: 3_1 

3_1.o: 3_1.c 
	$(CC) $(CFLAGS) -c 3_1.c 

3_1: 3_1.o
	$(CC) 3_1.o -o 3_1 $(THREADFLAGS) $(CFLAGS)

3_3.o: 3_3.c 
	$(CC) $(CFLAGS) -c 3_3.c 

3_3: 3_3.o
	$(CC) 3_3.o -o 3_3 $(THREADFLAGS) $(CFLAGS)

3_4.o: 3_4.c 
	$(CC) $(CFLAGS) -c 3_4.c 

3_4: 3_4.o
	$(CC) 3_4.o -o 3_4 $(THREADFLAGS) $(CFLAGS)

run_3_1: 3_1
	OMP_NUM_THREADS=4 ./3_1

run_3_3: 3_3
	./3_3 8

run_3_4: 3_4
	./3_4

clean:
	rm -f *.o 3_1 3_3 3_4