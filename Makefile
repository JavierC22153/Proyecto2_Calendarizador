CXX = g++

WX_CXXFLAGS = `wx-config --cxxflags`
WX_LIBS = `wx-config --libs`

CXXFLAGS = -std=c++17 -Wall -O2 $(WX_CXXFLAGS)
LDFLAGS = $(WX_LIBS) -pthread

TARGET = scheduler_simulator

SOURCES = main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Esto para sistemas basados en debian y que funcione la compilacion y dependencias
install-deps:
	sudo apt-get update
	sudo apt-get install -y libwxgtk3.0-gtk3-dev

.PHONY: all clean run install-deps
