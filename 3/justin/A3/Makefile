gccflags = -Wall -Wpedantic
hs = heat_seq
hp = heat_par

all: ${hs}.o ${hp}.o

${hs}.o: ${hs}.c
	gcc ${gccflags} -o ${hs}.o ${hs}.c

${hp}.o: ${hp}.c
	gcc ${gccflags} -o ${hp}.o ${hp}.c