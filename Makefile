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
TESTE_REATOR_SRCS = src/tests/teste_reator.cpp src/comum/reator.cpp

TESTE_REATOR_ENLACE_SRCS = src/tests/teste_reator_enlace.cpp src/comum/reator.cpp src/socket/udp_socket.cpp src/enlace/enlace.cpp src/enlace/checksum.cpp src/enlace/canal.cpp src/comum/buffer.cpp
TESTE_CANAL_ATRASO_SRCS = src/tests/teste_canal_atraso.cpp src/comum/reator.cpp src/socket/udp_socket.cpp src/enlace/enlace.cpp src/enlace/checksum.cpp src/enlace/canal.cpp src/comum/buffer.cpp

all: $(MAIN_EXEC) $(TEST_EXEC)

build/teste_canal_atraso: $(TESTE_CANAL_ATRASO_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_reator_enlace: $(TESTE_REATOR_ENLACE_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

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

build/teste_reator: $(TESTE_REATOR_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(MAIN_EXEC): $(MAIN_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_EXEC): $(TEST_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

teste_canal_atraso: build/teste_canal_atraso
	./build/teste_canal_atraso

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

teste_reator: build/teste_reator
	./build/teste_reator

teste_reator_enlace: build/teste_reator_enlace
	./build/teste_reator_enlace

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	rm -rf build/

debug: CXXFLAGS += -g -O0 -fsanitize=address,undefined
debug: $(MAIN_EXEC) $(TEST_EXEC)

release: CXXFLAGS += -O2 -DNDEBUG
release: $(MAIN_EXEC) $(TEST_EXEC)