CC = gcc
CCFLAGS = -Wall -Wpedantic
FILE = WuerfelRennen

all: $(FILE)

$(FILE): $(FILE).c
	$(CC) $(CCFLAGS) -o $(FILE).o $(FILE).c

clean:
	rm $(FILE).o