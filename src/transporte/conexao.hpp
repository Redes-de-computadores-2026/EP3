#pragma once
#include "segmento.hpp"
#include <cstdint>
#include <functional>

class CamadaTransporte;

// Essa tripla define cada conexão
struct ChaveConexao {
    uint16_t porta_local;
    uint16_t logico_remoto;
    uint16_t porta_remota;
    bool operator<(const ChaveConexao& o) const {
        return std::tie(porta_local, logico_remoto, porta_remota) < std::tie(o.porta_local, o.logico_remoto, o.porta_remota);
    }
};

class Conexao {
    ChaveConexao chave_;
    CamadaTransporte* transporte_;   // back-pointer pra delegar o envio
    std::function<void(const std::vector<uint8_t>&)> receber_callback;
    uint16_t porta_local_;
    uint16_t logico_remoto_;
    uint16_t porta_remota_;
    // subpasso 4-5: uint32_t prox_seq_, esperando_ack_; std::vector<uint8_t> em_voo_; etc
public:
    Conexao(ChaveConexao chave, CamadaTransporte* t, std::function<void(const std::vector<uint8_t>&)> callback);
    void enviar(const std::vector<uint8_t>& payload);
    void ao_receber(std::function<void(const std::vector<uint8_t>&)> callback);
    void entregar(const std::vector<uint8_t>& payload); // Transporte chama -> sobe pra app
    Conexao(const Conexao&) = delete; // previne copias com =
};
