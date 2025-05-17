CC = gcc
CCFLAGS = -Wall -Wpedantic
FILE = A2_1

all: $(FILE)

$(FILE): $(FILE).c
	$(CC) $(CCFLAGS) -o $(FILE).o $(FILE).c

clean:
	rm $(FILE).o