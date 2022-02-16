
CFLAGS =  -Wall -g -std=c++11

all: hello ltry

hello: hello.o
	g++ -o hello hello.o ${CFLAGS}

hello.o: hello.cpp
	g++ -c hello.cpp ${CFLAGS}

ltry: ltry.o
	g++ -o ltry ltry.o ${CFLAGS}

ltry.o: ltry.cpp
	g++ -c ltry.cpp ${CFLAGS}
