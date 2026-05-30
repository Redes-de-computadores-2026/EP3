#include "conexao.hpp"
#include "segmento.hpp"
#include "transporte.hpp"
#include <cstring>
#include <iostream>

Conexao::Conexao(ChaveConexao chave, CamadaTransporte* t, Reator* reator, std::function<void(const std::vector<uint8_t>&)> callback) :
    chave_(chave), transporte_(t), reator_(reator), receber_callback(callback) {}

void Conexao::agendar_retransmissao() {
    reator_->agendar(TIMEOUT_DELAY, [this]() {
        if (!aguardando_ack_) return;
        if (tentativas_ >= MAX_TENTATIVAS) {
            aguardando_ack_ = false;
            std::cerr << "[transporte] desistindo após " << MAX_TENTATIVAS << " tentativas\n";
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
    if (!aguardando_ack_) {
        iniciar_envio(payload);
    } else {
        fila_envio_.push(payload);
    }
}

void Conexao::entregar(const std::vector<uint8_t>& payload) {
    if (receber_callback) receber_callback(payload);
}

void Conexao::iniciar_envio(const std::vector<uint8_t>& payload) {
    prox_seq_++;
    aguardando_ack_ = true;
    em_voo_.resize(payload.size());
    memcpy(em_voo_.data(), payload.data(), payload.size());
    tentativas_ = 0;
    transporte_->_enviar_segmento(chave_, payload, DATA, prox_seq_, 0);
    agendar_retransmissao();
}

