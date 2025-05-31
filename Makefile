CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(shell wx-config --cxxflags)
LDFLAGS = $(shell wx-config --libs) -pthread

TARGET = scheduler_simulator

SOURCES = main.cpp \
          common.cpp \
          FIFO.cpp \
          SJF.cpp \
          SRT.cpp \
          Round_Robin.cpp \
          priority.cpp \
          mutex_simulator.cpp \
          semaforo_simulator.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp scheduler.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)


.PHONY: all clean run deps
