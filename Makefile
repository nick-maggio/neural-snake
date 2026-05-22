CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude -DUNICODE -D_UNICODE
LDFLAGS  = -ld3d11 -ldxgi -ld3dcompiler

# All .cpp files in src/ EXCEPT train_cli.cpp (which has its own main()).
SOURCES = $(filter-out src/train_cli.cpp, $(wildcard src/*.cpp))
OBJECTS = $(SOURCES:.cpp=.o)

TARGET = snake.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile any src/*.cpp into src/*.o
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-del /Q src\*.o 2>nul
	-del /Q $(TARGET) 2>nul

.PHONY: all clean