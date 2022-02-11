
CFLAGS =  -Wall -g -std=c++11

all: msg

msg: msg.o
	g++ -o msg msg.o

msg.o: msg.cpp
	g++ -c msg.cpp ${CFLAGS}
