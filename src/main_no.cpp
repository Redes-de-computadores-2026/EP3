#include "reator.hpp"
#include "udp_socket.hpp"
#include "enlace.hpp"
#include "canal.hpp"
#include "rotas.hpp"
#include "rede.hpp"
#include "transporte.hpp"
#include "aplicacao.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

// Porta de aplicação (LÓGICA) usada por todos os nós do chat.
static const uint16_t PORTA_APP = 7000;

// Demo de 2 nós. Uso: ./build/rede <1|2> [perda]
//   ./build/rede 1          -> no 1, canal perfeito
//   ./build/rede 2 0.3      -> no 2, 30% de perda no canal
int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Uso: " << argv[0] << " <1|2> [perda 0..1]\n";
        return 1;
    }
    int id = std::atoi(argv[1]);
    if (id != 1 && id != 2) {
        std::cerr << "Id do no deve ser 1 ou 2\n";
        return 1;
    }
    double p_perda = (argc == 3) ? std::stod(argv[2]) : 0.0;

    // --- parametros fixos deste no e do peer ---
    uint16_t meu_logico   = (id == 1) ? 1 : 2;
    uint16_t minha_fisica = (id == 1) ? 5001 : 5002;
    uint16_t peer_logico  = (id == 1) ? 2 : 1;

    Reator reator;

    // --- monta a pilha, de baixo pra cima ---
    UdpSocket sock;
    sock.udpBind(minha_fisica);

    CanalSimulado canal(p_perda, 0.0, 0.0, 0, 0, 42);   // só perda; resto zerado
    CamadaEnlace enlace(&sock, &reator, &canal);

    TabelaRotas tabela;
    tabela.inserir(1, "127.0.0.1", 5001);
    tabela.inserir(2, "127.0.0.1", 5002);
    CamadaRede rede(meu_logico, &tabela);

    CamadaTransporte transporte(PORTA_APP, &reator);

    rede.conectar_abaixo(&enlace);       enlace.conectar_acima(&rede);
    transporte.conectar_abaixo(&rede);   rede.conectar_acima(&transporte);

    // --- aplicacao (chat) ---
    CamadaAplicacao app(transporte, PORTA_APP, std::cout);

    // socket UDP -> sobe pela pilha
    reator.registrar_fd(sock.sock(), [&]() {
        auto rec = sock.receber(0);
        if (rec) enlace.receber(rec->bytes, rec->origem);
    });
    // teclado -> manda mensagem (ou /sair)
    reator.registrar_fd(0, [&]() {
        std::string linha;
        if (!std::getline(std::cin, linha) || linha == "/sair") { app.fechar(); reator.parar(); return; }
        app.enviar_texto(linha);
    });

    app.conectar(peer_logico, PORTA_APP);

    std::cout << "no " << meu_logico << " pronto (UDP " << minha_fisica
              << ", perda=" << p_perda << "). Digite e tecle Enter. /sair para sair.\n";

    reator.executar();
    return 0;
}
