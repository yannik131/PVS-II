CC = gcc 
CFLAGS = -O3 -Wall 
THREADFLAGS = -pthread 

all: 1_1 WuerfelRennen

1_1.o: 1_1.c
	$(CC) $(CFLAGS) -c 1_1.c

1_1: 1_1.o
	$(CC) 1_1.o -o 1_1 $(THREADFLAGS) $(CFLAGS)

WuerfelRennen.o: WuerfelRennen.c 
	$(CC) $(CFLAGS) -c WuerfelRennen.c 

WuerfelRennen: WuerfelRennen.o 
	$(CC) WuerfelRennen.o -o WuerfelRennen $(THREADFLAGS) $(CFLAGS)

clean:
	rm -f *.o 1_1 WuerfelRennen