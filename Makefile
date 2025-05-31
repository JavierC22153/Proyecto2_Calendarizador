CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -O2 `wx-config --cxxflags`
LDFLAGS   := `wx-config --libs`

SOURCES   := main.cpp \
             common.cpp \
             FIFO.cpp \
             SJF.cpp \
             SRT.cpp \
             Round_Robin.cpp \
             priority.cpp \
             mutex_simulator.cpp \
             semaforo_simulator.cpp

OBJECTS   := $(SOURCES:.cpp=.o)
EXECUTABLE:= scheduler_simulator

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean

