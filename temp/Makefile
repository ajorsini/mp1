
CFLAGS =  -Wall -g -std=c++11

all: test

test: test.o node.o op.o
	g++ -o test test.o node.o op.o ${CFLAGS}

test.o: test.cpp opnnode.h
	g++ -c test.cpp ${CFLAGS}

node.o: node.cpp opnnode.h
	g++ -c node.cpp ${CFLAGS}

op.o: op.cpp opnnode.h
	g++ -c op.cpp ${CFLAGS}


hello: hello.o
	g++ -o hello hello.o ${CFLAGS}

hello.o: hello.cpp
	g++ -c hello.cpp ${CFLAGS}

ltry: ltry.o
	g++ -o ltry ltry.o ${CFLAGS}

ltry.o: ltry.cpp
	g++ -c ltry.cpp ${CFLAGS}
