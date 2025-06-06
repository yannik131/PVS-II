gccflags = -Wall -Wpedantic -O3
file = radix_parallel

all: ${file}.o

${file}.o: ${file}.c
	gcc ${gccflags} -o ${file}.o ${file}.c

clear:
	rm ${file}.o