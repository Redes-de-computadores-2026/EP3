#include "transporte.hpp"
#include "segmento.hpp"
#include "camada.hpp"
#include "conexao.hpp"
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

    bool chamou = false;
    std::vector<uint8_t> recebido;
    Endereco e;
    e.logico = 1000;
    e.porta = 200;
    t.abrir(100, e, [&](const std::vector<uint8_t>& payload) {
        chamou = true;
        recebido = payload;
    });

    TransportHeader h;
    h.porta_destino = 100;
    h.porta_origem  = 200;
    h.flags         = DATA;
    std::vector<uint8_t> pdu(TAM_TRANSPORT_HEADER + 3);
    serializar_transport_header(h, pdu);
    pdu[13] = 0x10; pdu[14] = 0x20; pdu[15] = 0x30;

    Endereco origem;
    origem.logico = 1000;
    t.receber(pdu, origem);

    assert(chamou);
    assert(recebido.size() == 3);
    assert(recebido[0] == 0x10 && recebido[1] == 0x20 && recebido[2] == 0x30);

    std::cout << "Tudo certo!" << std::endl;

    return 0;
}
