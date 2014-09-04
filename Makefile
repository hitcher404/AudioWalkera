LDFLAGS=-lasound
CXXFLAGS= -O3

all: audiowalkera 

audiowalkera: audiowalkera.cpp

clean:
	rm -f audiowalkera  *.o
