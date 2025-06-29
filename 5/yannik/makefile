CC = gcc
CFLAGS = -Wall -O3
THREADFLAGS = -pthread

all: PrefixSumCalculator.class App.class

PrefixSumCalculator.class: PrefixSumCalculator.java
	javac PrefixSumCalculator.java

App.class: App.java
	javac App.java

clean:
	rm -f *.class 