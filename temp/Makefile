
CFLAGS =  -Wall -g -std=c++11

all: hello

hello: hello.o
	g++ -o hello hello.o ${CFLAGS}

hello.o: hello.cpp
	g++ -c hello.cpp ${CFLAGS}
