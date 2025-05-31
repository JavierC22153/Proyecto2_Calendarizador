# Makefile para Simulador de Algoritmos de Calendarización

# Compilador
CXX = g++

# Flags de wxWidgets
WX_CXXFLAGS = `wx-config --cxxflags`
WX_LIBS = `wx-config --libs`

# Flags adicionales
CXXFLAGS = -std=c++17 -Wall -O2 $(WX_CXXFLAGS)
LDFLAGS = $(WX_LIBS) -pthread

# Nombre del ejecutable
TARGET = scheduler_simulator

# Archivos fuente
SOURCES = main.cpp

# Archivos objeto
OBJECTS = $(SOURCES:.cpp=.o)

# Regla principal
all: $(TARGET)

# Compilar el ejecutable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compilar archivos objeto
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
	rm -f $(OBJECTS) $(TARGET)

# Ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Instalar dependencias (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libwxgtk3.0-gtk3-dev

# Phony targets
.PHONY: all clean run install-deps
