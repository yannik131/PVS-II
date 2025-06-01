CC = gcc 
CFLAGS = -O3 -Wall 
THREADFLAGS = -pthread 

all: 3_1 3_2 3_3 grid

3_1.o: 3_1.c 
	$(CC) $(CFLAGS) -c 3_1.c 

3_1: 3_1.o
	$(CC) 3_1.o -o 3_1 $(THREADFLAGS) $(CFLAGS)

3_2.o: 3_2.c 
	$(CC) $(CFLAGS) -c 3_2.c

3_2: 3_2.o 	
	$(CC) 3_2.o -o 3_2 $(THREADFLAGS) $(CFLAGS)

3_3.o: 3_3.c
	$(CC) $(CFLAGS) -c 3_3.c 

3_3: 3_3.o 
	$(CC) 3_3.o -o 3_3 $(THREADFLAGS) $(CFLAGS)

grid.o: grid.c
	$(CC) $(CFLAGS) -c grid.c 

grid: grid.o 
	$(CC) grid.o -o grid $(THREADFLAGS) $(CFLAGS)

clean:
	rm *.o 3_1 3_2 3_3 grid 