.PHONY: all clean debug release

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c++17
DEBUGFLAGS = -g -O0 -fsanitize=address,undefined

MAIN_EXEC=build/rede
MAIN_SRCS = src/main_no.cpp

all: $(MAIN_EXEC)

clean:
	rm -rf build/

debug: CXXFLAGS += -g -O0 -fsanitize=address,undefined
debug: $(MAIN_EXEC)

release: CXXFLAGS += -O2 -DNDEBUG
release: $(MAIN_EXEC)

$(MAIN_EXEC): $(MAIN_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(DEBUG_EXEC): $(MAIN_SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^
