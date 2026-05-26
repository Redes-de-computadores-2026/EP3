#include "reator.hpp"
#include "enlace.hpp"
#include "udp_socket.hpp"
#include "camada.hpp"
#include <vector>
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

    CamadaSpy spy_a;
    CamadaSpy spy_b;

    ena.conectar_acima(&spy_a);
    spy_a.conectar_abaixo(&ena);

    enb.conectar_acima(&spy_b);
    spy_b.conectar_abaixo(&enb);

    r.registrar_fd(sa.sock(), [&sa, &ena]() {
        auto r = sa.receber(0);
        if (r.has_value()) ena.receber(r->bytes, r->origem);
    });
    r.registrar_fd(sb.sock(), [&sb, &enb]() {
        auto r = sb.receber(0);
        if (r.has_value()) enb.receber(r->bytes, r->origem);
    });

    Endereco para_b;
    para_b.porta_fisica = 5002;
    para_b.ip_fisico = "127.0.0.1";

    std::vector<uint8_t> teste = {'h','i'};

    r.agendar(50, [&](){ ena.enviar({'h','i'}, para_b); });
    r.agendar(500, [&](){ r.parar(); });
    r.executar();

    assert(spy_b.mensagem_recebida);
    assert(spy_b.ultima_origem.porta_fisica == 5001);
    assert(spy_b.ultimo_payload_recebido == teste);

    spy_b.mensagem_recebida = false;
    spy_b.contador = 0;
    for (int i = 0; i < 5; i++) {
        r.agendar(50 + i*20, [&, i](){
            std::vector<uint8_t> msg = { uint8_t(i) };
            ena.enviar(msg, para_b);
        });
    }
    r.agendar(500, [&]() { r.parar(); });

    r.executar();

    std::cerr << "Contador: " << spy_b.contador << std::endl;
    assert(spy_b.contador == 5);

    std::cout << "Teste OK" << std::endl;

    return 0;
}
