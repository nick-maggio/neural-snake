CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
SOURCES = main.cpp game_state.cpp random_agent.cpp simulate.cpp
TARGET = snake.exe

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean