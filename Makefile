.PHONY: all clean debug release test

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c++17 -Isrc/comum -Isrc/socket -Isrc/enlace

MAIN_EXEC = build/rede
MAIN_SRCS = src/main_no.cpp src/socket/udp_socket.cpp

TEST_EXEC = build/teste
TEST_SRCS = src/tests/teste_camada.cpp src/socket/udp_socket.cpp
TESTE_CRC_SRCS = src/tests/teste_crc.cpp src/enlace/checksum.cpp
TESTE_BUFFER_SRCS = src/tests/teste_buffer.cpp src/comum/buffer.cpp

TESTE_ENLACE_SRCS = src/tests/teste_canal_enlace.cpp src/socket/udp_socket.cpp src/enlace/enlace.cpp src/enlace/canal.cpp src/enlace/checksum.cpp src/comum/buffer.cpp
TESTE_CANAL_SRCS = src/tests/teste_canal.cpp src/enlace/canal.cpp

all: $(MAIN_EXEC) $(TEST_EXEC)

build/teste_crc: $(TESTE_CRC_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_buffer: $(TESTE_BUFFER_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_enlace: $(TESTE_ENLACE_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_canal_enlace: $(TESTE_ENLACE_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_canal: $(TESTE_CANAL_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(MAIN_EXEC): $(MAIN_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_EXEC): $(TEST_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

teste_crc: build/teste_crc
	./build/teste_crc

teste_buffer: build/teste_buffer
	./build/teste_buffer

teste_enlace: build/teste_enlace
	./build/teste_enlace

teste_canal_enlace: build/teste_canal_enlace
	./build/teste_canal_enlace

teste_canal: build/teste_canal
	./build/teste_canal

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	rm -rf build/

debug: CXXFLAGS += -g -O0 -fsanitize=address,undefined
debug: $(MAIN_EXEC) $(TEST_EXEC)

release: CXXFLAGS += -O2 -DNDEBUG
release: $(MAIN_EXEC) $(TEST_EXEC)