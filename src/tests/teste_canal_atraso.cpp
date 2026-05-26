#include "reator.hpp"
#include "enlace.hpp"
#include "canal.hpp"
#include "quadro.hpp"
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <chrono>
#include <algorithm>

class CamadaSpy : public Camada {
public:
    bool mensagem_recebida = false;
    std::chrono::steady_clock::time_point t0;
    std::vector<uint8_t> ultimo_payload_recebido;
    Endereco ultima_origem;
    int contador = 0;
    std::vector<uint32_t> tempos_chegada;

    void enviar(const std::vector<uint8_t>&, const Endereco&) override {};

    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override {
        mensagem_recebida = true;
        ultima_origem = origem;
        ultimo_payload_recebido = pdu;
        contador++;
        auto tx = std::chrono::steady_clock::now() - t0;
        tempos_chegada.emplace_back(std::chrono::duration_cast<std::chrono::milliseconds>(tx).count());
    };

};

int main() {
    UdpSocket sa;
    UdpSocket sb;
    Reator r;

    sa.udpBind(5001);
    sb.udpBind(5002);

    CanalSimulado cs(0.0, 0.0, 1.0, 100, 200, 90);

    CamadaEnlace ena(&sa, &r, &cs);
    CamadaEnlace enb(&sb, &r);

    CamadaSpy spy_b;

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

    auto t0 = std::chrono::steady_clock::now();
    spy_b.t0 = t0;
    for (int i = 0; i < 5; i++) {
        ena.enviar({'h','i'}, para_b);
    }

    r.agendar(1200, [&]() {
        r.parar();
    });

    r.executar();

    assert(spy_b.contador == 5);
    assert(*std::min_element(spy_b.tempos_chegada.begin(), spy_b.tempos_chegada.end()) >= 100);
    for (uint32_t tx: spy_b.tempos_chegada) {
        std::cout << "Tempo de chegada= " << tx << "ms" << std::endl;
    }

    std::cout << "Testes OK!" << std::endl;

    return 0;
}