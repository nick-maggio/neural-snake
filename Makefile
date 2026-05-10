CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O2

SRCDIR  = src
SOURCES = $(SRCDIR)/main.cpp        \
          $(SRCDIR)/game_state.cpp  \
          $(SRCDIR)/simulate.cpp    \
          $(SRCDIR)/encode_state.cpp \
          $(SRCDIR)/neural_net.cpp  \
          $(SRCDIR)/neural_agent.cpp \
          $(SRCDIR)/random_agent.cpp \
          $(SRCDIR)/population.cpp

TARGET  = snake.exe

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -Iinclude $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
