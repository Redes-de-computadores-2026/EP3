#include "enlace.hpp"
#include "reator.hpp"
#include "rede.hpp"
#include "rotas.hpp"
#include "transporte.hpp"
#include "udp_socket.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <id_no>\n";
        return 1;
    }

    int id_no = std::stoi(argv[1]);
    if (id_no < 1 || id_no > 4) {
        std::cerr << "Id invalido. Use 1, 2, 3 ou 4.\n";
        return 1;
    }

    Reator reator;
    UdpSocket socket;
    socket.udpBind(5000 + id_no);

    CamadaEnlace enlace(&socket, &reator);

    TabelaRotas rotas;
    if (id_no == 1) {
        rotas.inserir(1, "127.0.0.1", 5001);
        rotas.inserir(2, "127.0.0.1", 5002);
        rotas.inserir(3, "127.0.0.1", 5003);
        rotas.inserir(4, "127.0.0.1", 5003); // vamos usar o 3 para chegar no lugar
    } else if (id_no == 2) {
        rotas.inserir(1, "127.0.0.1", 5001);
        rotas.inserir(2, "127.0.0.1", 5002);
        rotas.inserir(3, "127.0.0.1", 5003);
        rotas.inserir(4, "127.0.0.1", 5003); // vamos usar o 3 para chegar no lugar
    } else if (id_no == 3) {
        rotas.inserir(1, "127.0.0.1", 5001);
        rotas.inserir(2, "127.0.0.1", 5002);
        rotas.inserir(3, "127.0.0.1", 5003);
        rotas.inserir(4, "127.0.0.1", 5004);
    } else {
        rotas.inserir(1, "127.0.0.1", 5003); // vamos usar o 3 para chegar no lugar
        rotas.inserir(2, "127.0.0.1", 5003); // vamos usar o 3 para chegar no lugar
        rotas.inserir(3, "127.0.0.1", 5003);
        rotas.inserir(4, "127.0.0.1", 5004);
    }

    CamadaRede rede(id_no, &rotas);
    CamadaTransporte transporte(6000 + id_no, &reator);

    rede.conectar_abaixo(&enlace);
    enlace.conectar_acima(&rede);
    transporte.conectar_abaixo(&rede);
    rede.conectar_acima(&transporte);

    std::vector<Conexao*> conns(5, nullptr);
    for (int destino = 1; destino <= 4; ++destino) {
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
        if (destino < 1 || destino > 4 || destino == id_no || !conns[destino]) {
            std::cerr << "Destino invalido. Use 1..4 e diferente do seu.\n";

            return;
        }
        conns[destino]->enviar(std::vector<uint8_t>(msg.begin(), msg.end()));
    });

    std::cout << "No " << id_no << " pronto. Digite: <id> <mensagem> (ex: 3 oi) ou /sair.\n";
    reator.executar();
    return 0;
}
