#include "enlace.hpp"
#include "reator.hpp"
#include "rede.hpp"
#include "rotas.hpp"
#include "transporte.hpp"
#include "udp_socket.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

struct ConfigNo {
    uint16_t logico = 0;
    uint16_t porta_fisica = 0;
    uint16_t porta_transporte = 0;
    uint16_t logico_remoto = 0;
    uint16_t porta_transporte_remota = 0;
};

bool carregar_config(int id, ConfigNo& config) {
    if (id == 1) {
        config.logico = 1;
        config.porta_fisica = 5001;
        config.porta_transporte = 6001;
        config.logico_remoto = 2;
        config.porta_transporte_remota = 6002;
        return true;
    }
    if (id == 2) {
        config.logico = 2;
        config.porta_fisica = 5002;
        config.porta_transporte = 6002;
        config.logico_remoto = 1;
        config.porta_transporte_remota = 6001;
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <id_no>\n";
        std::cerr << "Exemplo: " << argv[0] << " 1\n";
        return 1;
    }

    int id_no = std::stoi(argv[1]);
    ConfigNo config;

    if (!carregar_config(id_no, config)) {
        std::cerr << "ID inválido. Use 1 ou 2.\n";
        return 1;
    }

    Reator reator;

    UdpSocket socket;
    socket.udpBind(config.porta_fisica);

    CamadaEnlace enlace(&socket, &reator);

    TabelaRotas rotas;
    rotas.inserir(1, "127.0.0.1", 5001);
    rotas.inserir(2, "127.0.0.1", 5002);

    CamadaRede rede(config.logico, &rotas);
    CamadaTransporte transporte(config.porta_transporte, &reator);

    rede.conectar_abaixo(&enlace);
    enlace.conectar_acima(&rede);
    transporte.conectar_abaixo(&rede);
    rede.conectar_acima(&transporte);

    Endereco destino;
    destino.logico = config.logico_remoto;
    destino.porta = config.porta_transporte_remota;

    Conexao& conn = transporte.abrir(config.porta_transporte, destino, [&](const std::vector<uint8_t>& msg) {
        std::string texto(msg.begin(), msg.end());
        std::cout << "[" << config.logico_remoto << "] " << texto << std::endl;
    });

    conn.gerenciar_erro([&]() {
        std::cerr << "Conexao encerrada por excesso de perdas.\n";
        reator.parar();
    });

    reator.registrar_fd(socket.sock(), [&]() {
        while (true) {
            auto rec = socket.receber(0);
            if (!rec) {
                break;
            }
            enlace.receber(rec->bytes, rec->origem);
        }
    });

    reator.registrar_fd(0, [&]() {
        std::string linha;
        if (!std::getline(std::cin, linha)) {
            reator.parar();
            return;
        }
        if (linha == "/sair") {
            reator.parar();
            return;
        }
        conn.enviar(std::vector<uint8_t>(linha.begin(), linha.end()));
    });

    std::cout << "No " << config.logico << " pronto. Digite mensagens ou /sair.\n";
    reator.executar();
    return 0;
}
