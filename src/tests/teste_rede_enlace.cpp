#include "reator.hpp"
#include "udp_socket.hpp"
#include "enlace.hpp"
#include "rotas.hpp"
#include "rede.hpp"
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

class CamadaSpy : public Camada {
public:
    bool mensagem_recebida = false;
    std::vector<uint8_t> ultimo_payload_recebido;
    Endereco ultima_origem;
    int contador = 0;

    void enviar(const std::vector<uint8_t>&, const Endereco&) override {};

    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override {
        mensagem_recebida = true;
        ultima_origem = origem;
        ultimo_payload_recebido = pdu;
        contador++;
    };

};

int main() {
    UdpSocket sa;
    UdpSocket sb;
    Reator r;

    sa.udpBind(5001);
    sb.udpBind(5002);
    CamadaEnlace ena(&sa, &r);
    CamadaEnlace enb(&sb, &r);

    TabelaRotas t1;
    TabelaRotas t2;
    t1.inserir(2, "127.0.0.1", 5002);
    t2.inserir(1, "127.0.0.1", 5001);

    CamadaRede rede1(1, &t1);
    CamadaRede rede2(2, &t2);

    CamadaSpy spy_a;
    CamadaSpy spy_b;

    ena.conectar_acima(&rede1);
    rede1.conectar_abaixo(&ena);
    rede1.conectar_acima(&spy_a);
    spy_a.conectar_abaixo(&rede1);

    enb.conectar_acima(&rede2);
    rede2.conectar_abaixo(&enb);
    rede2.conectar_acima(&spy_b);
    spy_b.conectar_abaixo(&rede2);

    r.registrar_fd(sa.sock(), [&](){
        auto rlinha = sa.receber(0);                  // não-bloqueante; o reator já garantiu dado
        if (rlinha) ena.receber(rlinha->bytes, rlinha->origem);
    });
    r.registrar_fd(sb.sock(), [&](){
        auto rlinha = sb.receber(0);
        if (rlinha) enb.receber(rlinha->bytes, rlinha->origem);
    });

     rede1.conectar_abaixo(&ena);
     ena.conectar_acima(&rede1);

    std::vector<uint8_t> msg = {'o','i'};
    Endereco dest;  dest.logico = 2;
    r.agendar(50,  [&](){ rede1.enviar(msg, dest); });
    r.agendar(300, [&](){ r.parar(); });
    r.executar();

    assert(spy_b.contador == 1);
    assert(spy_b.ultimo_payload_recebido == msg);
    assert(spy_b.ultima_origem.logico == 1);

    std::cout << "Teste concluidos com sucesso!" << std::endl;

    return 0;
}
