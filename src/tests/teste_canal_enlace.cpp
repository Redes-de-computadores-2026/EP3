
#include "camada.hpp"
#include "udp_socket.hpp"
#include "enlace.hpp"
#include "canal.hpp"
#include "reator.hpp"
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
    Reator reator;

    sa.udpBind(5001);
    sb.udpBind(5002);

    CanalSimulado cs(0.2, 0.1, 0.0, 0, 0, 90);

    CamadaEnlace ena(&sa, &reator, &cs);
    CamadaEnlace enb(&sb, &reator);
    CamadaSpy spy_b;

    enb.conectar_acima(&spy_b);
    spy_b.conectar_abaixo(&enb);

    Endereco para_b;
    para_b.ip_fisico = "127.0.0.1";
    para_b.porta_fisica = 5002;

    std::vector<uint8_t> tome = { 'o', 'l', 'a', ' ', 'm', 'u', 'n', 'd', 'o' };
    int enviadas = 0;
    int nao_chegaram = 0;
    int entregues = 0;
    int descartadas_por_crc = 0;
    for (int i = 0; i < 100; i++) {
        ena.enviar(tome, para_b);
        enviadas++;
        auto resposta = sb.receber(500);
        if (resposta.has_value()) {
            enb.receber(resposta->bytes, resposta->origem);
            if (spy_b.mensagem_recebida) entregues++;
            else descartadas_por_crc++;
        } else {
            nao_chegaram++;
        }
        spy_b.mensagem_recebida = false;
    }

    assert(enviadas == entregues + nao_chegaram + descartadas_por_crc);
    assert(nao_chegaram > 10 && nao_chegaram < 30);          // ~20 ± 10
    assert(descartadas_por_crc > 3 && descartadas_por_crc < 17); // ~10 ± 7

    std::cerr << "Enviadas: " << enviadas << std::endl;
    std::cerr << "Entregues: " << entregues << std::endl;
    std::cerr << "Perdidas: " << nao_chegaram << std::endl;
    std::cerr << "Corrompidas: " << descartadas_por_crc << std::endl;

    std::cerr << "Loopback OK!" << std::endl;

    return 0;
}
