#include "transporte.hpp"
#include "segmento.hpp"
#include "conexao.hpp"
#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <map>

CamadaTransporte::CamadaTransporte(uint16_t porta_local, Reator* r) : reator_(r), porta_local_(porta_local) {}

const Instrumentacao& CamadaTransporte::instrumentacao() const {
    return instr_;
}

Instrumentacao& CamadaTransporte::instrumentacao() {
    return instr_;
}

Conexao& CamadaTransporte::abrir(uint16_t porta_local, const Endereco& destino, std::function<void(const std::vector<uint8_t>&)> callback) {
    ChaveConexao cv;
    cv.logico_remoto = destino.logico;
    cv.porta_remota = destino.porta;
    cv.porta_local = porta_local;
    auto [it, inserted] = conexoes_.try_emplace(cv, cv, this, reator_, callback);
    return it->second;
}

// Nosso App não vai usar essa função, só _enviar_segmento.
void CamadaTransporte::enviar(const std::vector<uint8_t>& payload, const Endereco& destino) {
    TransportHeader h;
    h.porta_destino = destino.porta;
    h.porta_origem = porta_local_;
    h.flags = DATA;
    std::vector<uint8_t> buf(TAM_TRANSPORT_HEADER + payload.size());
    serializar_transport_header(h, buf);
    memcpy(buf.data() + TAM_TRANSPORT_HEADER, payload.data(), payload.size());
    registrar_envio_segmento(payload.size(), h.flags);
    if (abaixo) {
        abaixo->enviar(buf, destino);
    }    
}

void CamadaTransporte::receber(const std::vector<uint8_t>& pdu, const Endereco& origem) {
    if (!(pdu.size() >= TAM_TRANSPORT_HEADER)) {
        instr_.incrementar("segmentos_descartados_sem_cabecalho");
        std::cerr << "Tamanho do segmento é menor do que o necessário." << std::endl;
        return;
    }
    instr_.incrementar("segmentos_recebidos");
    instr_.incrementar("bytes_cabecalho_recebidos", TAM_TRANSPORT_HEADER);
    instr_.incrementar("bytes_payload_recebidos", static_cast<Instrumentacao::valor_t>(pdu.size() - TAM_TRANSPORT_HEADER));
    TransportHeader h = desserializar_transport_header(pdu);
    ChaveConexao cs;
    cs.porta_local = h.porta_destino;
    cs.logico_remoto = origem.logico;
    cs.porta_remota = h.porta_origem;
    auto it = conexoes_.find(cs);
    if (it == conexoes_.end()) {
        instr_.incrementar("segmentos_descartados_sem_conexao");
        std::cerr << "Não encontramos conexão da porta " << cs.porta_local << " para porta " << cs.porta_remota << std::endl;
        return;
    }
    if (h.flags & ACK) {
        it->second.tratar_ack(h.ack_num);
        return;
    }
    if (h.flags & DATA) {
        std::vector<uint8_t> payload(pdu.size() - TAM_TRANSPORT_HEADER);
        memcpy(payload.data(), pdu.data() + TAM_TRANSPORT_HEADER, pdu.size() - TAM_TRANSPORT_HEADER);
        it->second.tratar_dado(h.seq_num, payload);
    }
}

void CamadaTransporte::_enviar_segmento(const ChaveConexao& chave, const std::vector<uint8_t>& payload, uint8_t flags, uint32_t seq, uint32_t ack) {
    TransportHeader h;
    h.ack_num = ack;
    h.seq_num = seq;
    h.flags = flags;
    h.porta_destino = chave.porta_remota;
    h.porta_origem = chave.porta_local;
    std::vector<uint8_t> buf(TAM_TRANSPORT_HEADER + payload.size());
    serializar_transport_header(h, buf);
    memcpy(buf.data() + TAM_TRANSPORT_HEADER, payload.data(), payload.size());
    Endereco d;
    d.logico = chave.logico_remoto;
    d.porta = chave.porta_remota;
    registrar_envio_segmento(payload.size(), flags);
    if (abaixo) {
        abaixo->enviar(buf, d);
    }
}

void CamadaTransporte::registrar_envio_segmento(std::size_t payload_size, uint8_t flags) {
    instr_.incrementar("segmentos_enviados");
    instr_.incrementar("bytes_cabecalho_enviados", TAM_TRANSPORT_HEADER);
    instr_.incrementar("bytes_payload_enviados", static_cast<Instrumentacao::valor_t>(payload_size));
    if (flags & ACK) {
        instr_.incrementar("acks_enviados");
    }
    if (flags & DATA) {
        instr_.incrementar("dados_enviados");
    }
}
