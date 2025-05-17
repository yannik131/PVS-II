file = B1A1
sourcefile = $(file).c
outfile = $(file).o

all: $(outfile)

$(outfile): $(sourcefile)
	gcc -Wall -Wpedantic $(sourcefile) -o $(outfile)

clean:
	rm $(outfile)