CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c++17

MAIN_EXEC=build/rede
MAIN_SRCS = src/main_no.cpp

all: $(MAIN_EXEC)

$(MAIN_EXEC): $(MAIN_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: all clean
