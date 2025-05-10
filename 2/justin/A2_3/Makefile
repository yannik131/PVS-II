CC = gcc
CCFLAGS = -Wall -Wpedantic
FILE1 = A2_3
FILE2 = vorlesung

all: $(FILE1) $(FILE2)

$(FILE1): $(FILE1).c
	$(CC) $(CCFLAGS) -o $(FILE1).o $(FILE1).c

$(FILE2): $(FILE2).c
	$(CC) $(CCFLAGS) -o $(FILE2).o $(FILE2).c

clean:
	rm $(FILE1).o
	rm $(FILE2).o