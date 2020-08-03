OBJ = config.o server.o main.o

all : $(OBJ)
	g++ -std=c++11 -o demo $(OBJ) -pthread

config.o : config.h config.cpp
	g++ -std=c++11 -c -o config.o config.cpp

server.o : server.h server.cpp
	g++ -std=c++11 -c -o server.o server.cpp

main.o : main.cpp
	g++ -std=c++11 -c -o main.o main.cpp

clean :
	rm -f demo *.o

