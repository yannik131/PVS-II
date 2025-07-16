CC = gcc
CFLAGS = -Wall -O3
THREADFLAGS = -pthread

all: rumor_simulation

rumor_simulation.o: rumor_simulation.c
	$(CC) $(CFLAGS) -c rumor_simulation.c

rumor_simulation: rumor_simulation.o
	$(CC) rumor_simulation.o -o rumor_simulation $(THREADFLAGS) $(CFLAGS)