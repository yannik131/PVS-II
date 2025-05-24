gccflags = -Wall -Wpedantic
st = search_tree
stm = search_tree_mod

all: ${st} ${stm}

${st}: ${st}.c
	gcc ${gccflags} -o ${st}.o ${st}.c

${stm}: ${stm}.c
	gcc ${gccflags} -o ${stm}.o ${stm}.c