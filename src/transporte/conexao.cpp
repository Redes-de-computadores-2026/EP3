#include "conexao.hpp"
#include "segmento.hpp"
#include "transporte.hpp"
#include <cstring>
#include <iostream>
#include <queue>

Conexao::Conexao(ChaveConexao chave, CamadaTransporte* t, Reator* reator, std::function<void(const std::vector<uint8_t>&)> callback) :
    chave_(chave), transporte_(t), reator_(reator), receber_callback_(callback) {}

void Conexao::agendar_retransmissao() {
    uint32_t minha_geracao = geracao_;
    reator_->agendar(TIMEOUT_DELAY, [this, minha_geracao]() {
        // o minha_geracao é uma copia congelada do geracao_ no momento do agendamento
         if (minha_geracao != geracao_) return; // so retransmite se ainda fizer sentido: se a conexao ainda esta naquela mesma mensagem!
        if (!aguardando_ack_) return;
        if (tentativas_ >= MAX_TENTATIVAS) {
            aguardando_ack_ = false;
            ativa_ = false;
            fila_envio_ = std::queue<std::vector<uint8_t>>();
            std::cerr << "[transporte] conexao morta após " << MAX_TENTATIVAS << " tentativas" << std::endl;
             if (erro_callback) erro_callback();
            return;
        }
        transporte_->_enviar_segmento(chave_, em_voo_, DATA, prox_seq_, 0);
        tentativas_++;
        agendar_retransmissao();
    });
}

void Conexao::tratar_ack(const uint32_t ack) {
    if (ack != prox_seq_ + 1) return;
    aguardando_ack_ = false;
    em_voo_.clear();
    if (!fila_envio_.empty()) {
        auto proximo = fila_envio_.front();
        fila_envio_.pop();
        iniciar_envio(proximo);
    }
}

void Conexao::tratar_dado(const uint32_t seq, const std::vector<uint8_t>& payload) {
    if (seq == prox_seq_esperado_) {
        entregar(payload);
        prox_seq_esperado_++;
    }
    transporte_->_enviar_segmento(chave_, {}, ACK, 0, prox_seq_esperado_);
}


void Conexao::enviar(const std::vector<uint8_t>& payload) {
     if (!ativa_) {
        std::cerr << "[transporte] conexão morta, envio rejeitado" << std::endl;
        return;
    }
    if (!aguardando_ack_) {
        iniciar_envio(payload);
    } else {
        fila_envio_.push(payload);
    }
}

void Conexao::entregar(const std::vector<uint8_t>& payload) {
    if (receber_callback_) receber_callback_(payload);
}

void Conexao::iniciar_envio(const std::vector<uint8_t>& payload) {
    geracao_++;
    prox_seq_++;
    aguardando_ack_ = true;
    em_voo_ = payload;
    tentativas_ = 0;
    transporte_->_enviar_segmento(chave_, payload, DATA, prox_seq_, 0);
    agendar_retransmissao();
}

void Conexao::ao_receber(std::function<void(const std::vector<uint8_t>&)> callback) {
    receber_callback_ = callback;
}

void Conexao::gerenciar_erro(std::function<void()> callback) {
    erro_callback = callback;
}

