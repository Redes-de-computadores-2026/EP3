.PHONY: all clean debug release test \
        teste_buffer teste_camada teste_crc teste_enlace \
        teste_canal teste_canal_enlace teste_reator \
        teste_reator_enlace teste_canal_atraso teste_rede \
        teste_rotas teste_segmento teste_transporte \
        experimento_perda

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c++17 -Isrc/comum -Isrc/socket -Isrc/enlace -Isrc/rede -Isrc/transporte -Isrc/aplicacao

MAIN_EXEC = build/rede
MAIN_SRCS = src/main_no.cpp src/socket/udp_socket.cpp src/rede/rede.cpp src/rede/rotas.cpp src/comum/buffer.cpp src/comum/instrumentacao/instrumentacao.cpp

TEST_EXEC = build/teste
TEST_SRCS = src/tests/teste_camada.cpp src/socket/udp_socket.cpp

TESTE_CRC_SRCS    = src/tests/teste_crc.cpp src/enlace/checksum.cpp
TESTE_BUFFER_SRCS = src/tests/teste_buffer.cpp src/comum/buffer.cpp
TESTE_CANAL_SRCS  = src/tests/teste_canal.cpp src/enlace/canal.cpp src/comum/instrumentacao/instrumentacao.cpp
TESTE_REATOR_SRCS = src/tests/teste_reator.cpp src/comum/reator.cpp

TESTE_ENLACE_SRCS = src/tests/teste_canal_enlace.cpp src/socket/udp_socket.cpp \
                    src/enlace/enlace.cpp src/enlace/canal.cpp \
                    src/enlace/checksum.cpp src/comum/buffer.cpp src/comum/reator.cpp \
                    src/comum/instrumentacao/instrumentacao.cpp

TESTE_REATOR_ENLACE_SRCS = src/tests/teste_reator_enlace.cpp src/comum/reator.cpp \
                            src/socket/udp_socket.cpp src/enlace/enlace.cpp \
                            src/enlace/checksum.cpp src/enlace/canal.cpp \
                            src/comum/buffer.cpp src/comum/instrumentacao/instrumentacao.cpp

TESTE_CANAL_ATRASO_SRCS = src/tests/teste_canal_atraso.cpp src/comum/reator.cpp \
                          src/socket/udp_socket.cpp src/enlace/enlace.cpp \
                          src/enlace/checksum.cpp src/enlace/canal.cpp \
                          src/comum/buffer.cpp src/comum/instrumentacao/instrumentacao.cpp

TESTE_REDE_SRCS = tests/rede/unitario/main.cpp src/rede/rede.cpp src/rede/rotas.cpp src/comum/buffer.cpp src/comum/instrumentacao/instrumentacao.cpp
TESTE_ROTAS_SRCS = src/tests/teste_rotas.cpp src/rede/rotas.cpp
TESTE_APLICACAO_SRCS = src/tests/teste_aplicacao.cpp \
						src/rede/rede.cpp src/comum/reator.cpp \
						src/transporte/transporte.cpp \
						src/aplicacao/aplicacao.cpp \
						src/enlace/enlace.cpp \
						src/enlace/canal.cpp

TESTE_REDE_ENLACE_SRCS = src/tests/teste_rede_enlace.cpp \
    src/rede/rede.cpp src/rede/rotas.cpp \
    src/enlace/enlace.cpp src/enlace/checksum.cpp src/enlace/canal.cpp \
    src/socket/udp_socket.cpp src/comum/buffer.cpp src/comum/reator.cpp \
    src/comum/instrumentacao/instrumentacao.cpp

## CAMADA TRANSPORTE: ##
TESTE_SEGMENTO_SRCS = src/tests/teste_segmento.cpp src/comum/buffer.cpp
TESTE_APLICACAO_SRCS = src/tests/teste_aplicacao.cpp \
                         src/aplicacao/aplicacao.cpp \
                         src/transporte/transporte.cpp src/transporte/conexao.cpp \
                         src/rede/rede.cpp src/rede/rotas.cpp \
                         src/enlace/enlace.cpp src/enlace/checksum.cpp src/enlace/canal.cpp \
                         src/socket/udp_socket.cpp \
                         src/comum/buffer.cpp src/comum/reator.cpp
TESTE_TRANSPORTE_SRCS = src/tests/teste_transporte.cpp src/transporte/transporte.cpp src/transporte/conexao.cpp src/comum/buffer.cpp src/comum/reator.cpp src/comum/instrumentacao/instrumentacao.cpp

EXPERIMENTO_PERDA_SRCS = experimentos/perda.cpp \
    src/socket/udp_socket.cpp src/enlace/enlace.cpp src/enlace/canal.cpp \
    src/enlace/checksum.cpp src/rede/rede.cpp src/rede/rotas.cpp \
    src/transporte/transporte.cpp src/transporte/conexao.cpp \
    src/comum/buffer.cpp src/comum/reator.cpp src/comum/instrumentacao/instrumentacao.cpp

all: $(MAIN_EXEC) $(TEST_EXEC)

build/teste_rede_enlace: $(TESTE_REDE_ENLACE_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_aplicacao: $(TESTE_APLICACAO_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_transporte: $(TESTE_TRANSPORTE_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/experimento_perda: $(EXPERIMENTO_PERDA_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_segmento: $(TESTE_SEGMENTO_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(MAIN_EXEC): $(MAIN_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_EXEC): $(TEST_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

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

build/teste_rede: $(TESTE_REDE_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/teste_rotas: $(TESTE_ROTAS_SRCS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

teste_rotas: build/teste_rotas
	./build/teste_rotas

teste_aplicacao: build/teste_aplicacao
	./build/teste_aplicacao

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

teste_segmento: build/teste_segmento
	./build/teste_segmento

teste_transporte: build/teste_transporte
	./build/teste_transporte

experimento_perda: build/experimento_perda
	./build/experimento_perda

teste_rede: build/teste_rede
	./build/teste_rede

teste_rede_enlace: build/teste_rede_enlace
	./build/teste_rede_enlace

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	rm -rf build/

debug: CXXFLAGS += -g -O0 -fsanitize=address,undefined
debug: $(MAIN_EXEC) $(TEST_EXEC)

release: CXXFLAGS += -O2 -DNDEBUG
release: $(MAIN_EXEC) $(TEST_EXEC)
