CC=g++
C11 = -std=c++11
CFLAGS=-Iinclude -O2 -Wall -D_GLIBCXX_ISE_CXX11_ABI=1
# LDFLAGS=-Llib -lDetailPlace -lGlobalPlace -lLegalizer -lPlacement -lParser -lPlaceCommon
SOURCES=src/main.cpp src/Placement.cpp src/Legalization.cpp gsrc/GlobalPlacer.cpp gsrc/ExampleFunction.cpp gsrc/GradSolver.cpp gsrc/partition/Partitioner.cpp PD_F/src/DetailPlace1.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=place

all: $(SOURCES) bin/$(EXECUTABLE)
	
bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(C11) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c ${INCLUDES}
	$(CC) $(C11) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)