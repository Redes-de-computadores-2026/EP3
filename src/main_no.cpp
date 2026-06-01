#include "enlace.hpp"
#include "reator.hpp"
#include "rede.hpp"
#include "rotas.hpp"
#include "transporte.hpp"
#include "udp_socket.hpp"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define MAX_CLIENTES 1000

TabelaRotas construir_rotas() {
  TabelaRotas rotas;
  for (int i = 1; i <= MAX_CLIENTES; ++i) {
    rotas.inserir(i, "127.0.0.1", 5000 + i);
  }
  return rotas;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <id_no>\n";
        return 1;
    }

    int id_no = std::stoi(argv[1]);

    Reator reator;
    UdpSocket socket;
    auto res = socket.udpBind(5000 + id_no);
    if (!res) {
        std::cerr << "Erro ao criar socket para o no " << id_no << ". Porta 5000+" << id_no << " pode estar em uso.\n";
        return 1;
    }

    CamadaEnlace enlace(&socket, &reator);
    
    auto rotas = construir_rotas();

    CamadaRede rede(id_no, &rotas);
    CamadaTransporte transporte(6000 + id_no, &reator);

    rede.conectar_abaixo(&enlace);
    enlace.conectar_acima(&rede);
    transporte.conectar_abaixo(&rede);
    rede.conectar_acima(&transporte);

    std::vector<Conexao*> conns(MAX_CLIENTES, nullptr);
    for (int destino = 1; destino <= MAX_CLIENTES; ++destino) {
        if (destino == id_no) continue;
        Endereco remoto;
        remoto.logico = destino;
        remoto.porta = 6000 + destino;
        Conexao& conn = transporte.abrir(6000 + id_no, remoto, [destino](const std::vector<uint8_t>& msg) {
            std::string texto(msg.begin(), msg.end());
            std::cout << "[" << destino << "] " << texto << std::endl;
        });
        conn.gerenciar_erro([destino]() {
            std::cerr << "Conexao com no " << destino << " encerrada por excesso de perdas.\n";
        });
        conns[destino] = &conn;
    }

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
        std::istringstream iss(linha);
        int destino = 0;
        if (!(iss >> destino)) {
            std::cerr << "Uso: <id> <mensagem> (ex: 3 oi) ou /sair.\n";
            return;
        }

        std::string msg;
        std::getline(iss, msg);
        if (!msg.empty() && msg.front() == ' ') msg.erase(0, 1);
        if (!conns[destino]) {
            std::cerr << "Destino invalido.\n";
            return;
        }
        conns[destino]->enviar(std::vector<uint8_t>(msg.begin(), msg.end()));
    });

    std::cout << "No " << id_no << " pronto. Digite: <id> <mensagem> (ex: 3 oi) ou /sair.\n";
    reator.executar();
    return 0;
}
