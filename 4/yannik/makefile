CC = gcc
CFLAGS = -Wall -O3
THREADFLAGS = -pthread

all: 4_1 barrier_semaphore consumer_producer

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

vector.o: vector.c
	$(CC) $(CFLAGS) -c vector.c

4_1.o: 4_1.c
	$(CC) $(CFLAGS) -c 4_1.c

4_1: util.o vector.o 4_1.o
	$(CC) util.o vector.o 4_1.o -o 4_1 $(CFLAGS)

barrier_semaphore.o: barrier_semaphore.c
	$(CC) $(CFLAGS) -c barrier_semaphore.c

barrier_semaphore: barrier_semaphore.o
	$(CC) barrier_semaphore.o -o barrier_semaphore $(CFLAGS)

consumer_producer.o: consumer_producer.c
	$(CC) $(CFLAGS) -c consumer_producer.c

consumer_producer: consumer_producer.o
	$(CC) consumer_producer.o -o consumer_producer $(CFLAGS)

clean:
	rm -f *.o 4_1 barrier_semaphore consumer_producer