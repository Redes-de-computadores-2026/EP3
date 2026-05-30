#pragma once
#include "segmento.hpp"
#include "reator.hpp"
#include <cstdint>
#include <functional>
#include <queue>

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
    ChaveConexao chave_; // id da conexao
    // configuracao da camada
    int MAX_TENTATIVAS = 5;
    uint32_t TIMEOUT_DELAY = 100;

    CamadaTransporte* transporte_;   // pointer pra delegar o envio
    Reator* reator_;
    
    std::function<void(const std::vector<uint8_t>&)> receber_callback_;
    
    // estado da conexao
    std::queue<std::vector<uint8_t>> fila_envio_;
    uint32_t geracao_ = 0; // qual envio
    bool aguardando_ack_ = false;
    uint32_t prox_seq_ = 0;
    std::vector<uint8_t> em_voo_; // copia do ultimo para retransmissao
    int tentativas_ = 0;
    bool ativa_ = true;
    std::function<void()> erro_callback;

    // receptor
    uint32_t prox_seq_esperado_ = 1;
public:
    Conexao(ChaveConexao chave, CamadaTransporte* t, Reator* r, std::function<void(const std::vector<uint8_t>&)> callback);
    void tratar_ack(const uint32_t ack);
    void tratar_dado(const uint32_t seq, const std::vector<uint8_t>& payload);
    void enviar(const std::vector<uint8_t>& payload);
    void ao_receber(std::function<void(const std::vector<uint8_t>&)> callback);
    void entregar(const std::vector<uint8_t>& payload); // Transporte chama -> sobe pra app
    void gerenciar_erro(std::function<void()> callback);
    Conexao(const Conexao&) = delete; // previne copias com =


private: 
    void iniciar_envio(const std::vector<uint8_t>& payload);
    void agendar_retransmissao();
};
