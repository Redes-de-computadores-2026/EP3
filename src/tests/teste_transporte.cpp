#include "transporte.hpp"
#include "segmento.hpp"
#include "camada.hpp"
#include <iostream>
#include <cassert>

class CamadaSpy : public Camada {
public:
    int envios = 0;
    int recebidos = 0;
    std::vector<uint8_t> ultimo_payload_recebido;
    std::vector<uint8_t> ultimo_payload_enviado;
    Endereco ultimo_destino;
    Endereco ultima_origem;


    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override {
        envios++;
        ultimo_payload_enviado = payload;
        ultimo_destino = destino;
    };

    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override {
        recebidos++;
        ultimo_payload_recebido = pdu;
        ultima_origem = origem;
    };
};

int main() {
    CamadaTransporte t(100);

    CamadaSpy spy_a;

    t.conectar_abaixo(&spy_a);
    spy_a.conectar_acima(&t);

    Endereco d;
    d.logico = 5;
    d.porta = 200;

    t.enviar({1, 2, 3}, d);
    assert(spy_a.envios == 1);
    assert(spy_a.ultimo_payload_enviado.size() == TAM_TRANSPORT_HEADER + 3);
    assert(spy_a.ultimo_payload_enviado[0] == 0 && spy_a.ultimo_payload_enviado[1] == 100);   // porta_origem=100 big-endian
    assert(spy_a.ultimo_payload_enviado[2] == 0 && spy_a.ultimo_payload_enviado[3] == 200);   // porta_destino=200
    assert(spy_a.ultimo_payload_enviado[12] == DATA);
    assert(spy_a.ultimo_payload_enviado[13]==1 && spy_a.ultimo_payload_enviado[14]==2 && spy_a.ultimo_payload_enviado[15]==3);
    assert(spy_a.ultimo_destino.logico == 5);

    CamadaSpy spy_b;
    t.conectar_acima(&spy_b);
    spy_b.conectar_abaixo(&t);

    Endereco e;
    e.logico = 1000;
    e.porta = 6969;

    TransportHeader h;
    h.porta_destino = 100;
    h.porta_origem = 200;
    h.flags = DATA;

    std::vector<uint8_t> pdu(TAM_TRANSPORT_HEADER + 3);
    serializar_transport_header(h, pdu);
    pdu[13] = 0x10;
    pdu[14] = 0x20;
    pdu[15] = 0x30;
    t.receber(pdu, e);
    assert(spy_b.recebidos == 1);
    assert(spy_b.ultimo_payload_recebido.size() == 3);
    assert(spy_b.ultimo_payload_recebido[0] == 0x10);
    assert(spy_b.ultimo_payload_recebido[1] == 0x20);
    assert(spy_b.ultimo_payload_recebido[2] == 0x30);
    assert(spy_b.ultima_origem.porta == 200);

    std::cout << "Teste OK!\n" << std::endl;


    return 0;
}
