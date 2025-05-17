CC = gcc 
CFLAGS = -O3 -Wall 
THREADFLAGS = -pthread 

all: 1_1 WuerfelRennen 1_3 1_3_cond

1_1.o: 1_1.c
	$(CC) $(CFLAGS) -c 1_1.c

1_1: 1_1.o
	$(CC) 1_1.o -o 1_1 $(THREADFLAGS) $(CFLAGS)

WuerfelRennen.o: WuerfelRennen.c 
	$(CC) $(CFLAGS) -c WuerfelRennen.c 

WuerfelRennen: WuerfelRennen.o 
	$(CC) WuerfelRennen.o -o WuerfelRennen $(THREADFLAGS) $(CFLAGS)

1_3.o: 1_3.c 
	$(CC) $(CFLAGS) -c 1_3.c 

1_3: 1_3.o 
	$(CC) 1_3.o -o 1_3 $(THREADFLAGS) $(CFLAGS)

1_3_cond.o: 1_3_cond.c
	$(CC) $(CFLAGS) -c 1_3_cond.c 

1_3_cond: 1_3_cond.o 
	$(CC) 1_3_cond.o -o 1_3_cond $(THREADFLAGS) $(CFLAGS)

clean:
	rm -f *.o 1_1 WuerfelRennen 1_3 1_3_cond