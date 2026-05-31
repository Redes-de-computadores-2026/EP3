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
    Reator r;
    CamadaTransporte t(100, &r);
    CamadaSpy teste1;

    // Teste de conexao morta e retransmissao
    Endereco e;
    e.logico = 5;
    e.porta = 200;

    bool erro_disparou = false;
    Conexao& c = t.abrir(100, e, [](auto&){});
    t.conectar_abaixo(&teste1);
    teste1.conectar_acima(&t);
    c.gerenciar_erro([&](){ erro_disparou = true; });

    c.enviar({1});
    r.agendar(700, [&](){ r.parar(); });
    r.executar();
    
    assert(teste1.envios == 6);
    assert(erro_disparou);
    c.enviar({9});
    assert(teste1.envios == 6);

    
    Endereco e2;
    e2.logico = 6;
    e2.porta = 201;
    Conexao& c2 = t.abrir(101, e2, [](auto&){});
    CamadaSpy teste2;

    t.conectar_abaixo(&teste2);
    teste2.conectar_acima(&t);

    c2.enviar({1});                                // sp.envios=1, timer @100
    r.agendar(150, [&](){ r.parar(); });
    r.executar();                                 // @100 retransmite → sp.envios=2
    assert(teste2.envios == 2);                       // houve 1 retransmissão

    TransportHeader ack; ack.porta_destino=101; ack.porta_origem=201; ack.flags=ACK; ack.ack_num=2;
    std::vector<uint8_t> ack_pdu(TAM_TRANSPORT_HEADER);
    serializar_transport_header(ack, ack_pdu);
    t.receber(ack_pdu, e2);

    r.agendar(250, [&](){ r.parar(); });
    r.executar();
    assert(teste2.envios == 2);

    // Teste de deduplicação:

    int n_entregas = 0;
    Endereco e3;
    e3.logico = 7;
    e3.porta = 202;
    t.abrir(103, e3, [&](const std::vector<uint8_t>&){ n_entregas++; });

    TransportHeader ack2; ack2.porta_destino=103; ack2.porta_origem=202; ack2.flags=DATA; ack2.seq_num=1;
    std::vector<uint8_t> pdu_data(TAM_TRANSPORT_HEADER + 3);
    serializar_transport_header(ack2, pdu_data);

    t.receber(pdu_data, e3);
    t.receber(pdu_data, e3);
    assert(n_entregas == 1);

    return 0;
}
