.PHONY: all clean debug release test

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c++17 -Isrc/comum -Isrc/socket

MAIN_EXEC = build/rede
MAIN_SRCS = src/main_no.cpp src/socket/udp_socket.cpp

TEST_EXEC = build/teste
TEST_SRCS = src/tests/teste_camada.cpp src/socket/udp_socket.cpp

all: $(MAIN_EXEC) $(TEST_EXEC)

$(MAIN_EXEC): $(MAIN_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_EXEC): $(TEST_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	rm -rf build/

debug: CXXFLAGS += -g -O0 -fsanitize=address,undefined
debug: $(MAIN_EXEC) $(TEST_EXEC)

release: CXXFLAGS += -O2 -DNDEBUG
release: $(MAIN_EXEC) $(TEST_EXEC)