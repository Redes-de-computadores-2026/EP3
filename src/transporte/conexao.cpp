#include "conexao.hpp"
#include "segmento.hpp"
#include "transporte.hpp"

Conexao::Conexao(ChaveConexao chave, CamadaTransporte* t, std::function<void(const std::vector<uint8_t>&)> callback) :
    chave_(chave), transporte_(t), receber_callback(callback) {}

void Conexao::enviar(const std::vector<uint8_t>& payload) {
    transporte_->_enviar_segmento(chave_, payload, DATA, 0, 0);
}

void Conexao::entregar(const std::vector<uint8_t>& payload) {
    if (receber_callback) receber_callback(payload);
}

