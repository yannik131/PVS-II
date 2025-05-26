gccflags = -Wall -Wpedantic
st = search_tree
stm = search_tree_mod

all: ${st}.o ${stm}.o

${st}.o: ${st}.c
	gcc ${gccflags} -o ${st}.o ${st}.c

${stm}.o: ${stm}.c
	gcc ${gccflags} -o ${stm}.o ${stm}.c