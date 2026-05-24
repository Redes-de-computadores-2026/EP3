
#include "camada.hpp"
#include "udp_socket.hpp"
#include "enlace.hpp"
#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>

class CamadaSpy : public Camada {
public:
    bool mensagem_recebida = false;
    std::vector<uint8_t> ultimo_payload_recebido;
    Endereco ultima_origem;

    void enviar(const std::vector<uint8_t>&, const Endereco&) override {};

    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override {
        mensagem_recebida = true;
        ultima_origem = origem;
        ultimo_payload_recebido = pdu;
    };

};

int main() {
    UdpSocket sa;
    UdpSocket sb;

    sa.udpBind(5001);
    sb.udpBind(5002);

    CamadaEnlace ena(&sa);
    CamadaEnlace enb(&sb);

    CamadaSpy spy_a;
    CamadaSpy spy_b;

    ena.conectar_acima(&spy_a);
    spy_a.conectar_abaixo(&ena);

    enb.conectar_acima(&spy_b);
    spy_b.conectar_abaixo(&enb);

    Endereco para_b;
    para_b.ip_fisico = "127.0.0.1";
    para_b.porta_fisica = 5002;

    std::vector<uint8_t> tome = { 'o' };
    ena.enviar(tome, para_b);

    auto resposta = sb.receber(500);
    assert(resposta.has_value());
    enb.receber(resposta->bytes, resposta->origem);
    assert(spy_b.mensagem_recebida);
    assert(spy_b.ultimo_payload_recebido == tome);
    assert(spy_b.ultima_origem.porta_fisica == 5001);
    std::cerr << "Loopback OK!" << std::endl;

    std::vector<uint8_t> xesquedidi = resposta->bytes;
    xesquedidi[4] ^= 0x01;
    spy_b.mensagem_recebida = false;
    enb.receber(xesquedidi, resposta->origem);
    assert(spy_b.mensagem_recebida == false);
    std::cerr << "Enlace pegou a corrupcao em BRB!" << std::endl;

    return 0;
}
