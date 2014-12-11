CC 			= g++
CXXFLAGS 	= -c -std=c++11 -O2
#LDFLAGS 	= -lasound -pthread
LDFLAGS 	= -lasound
SOURCES 	= audiowalkera.cpp main.cpp
OBJECTS 	= $(SOURCES:.cpp=.o)
EXECUTABLE 	= audiowalkera

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE) $(OBJECTS) 
