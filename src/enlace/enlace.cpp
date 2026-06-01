#include "enlace.hpp"
#include "quadro.hpp"
#include "canal.hpp"
#include "checksum.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstring>

CamadaEnlace::CamadaEnlace(UdpSocket* socket, Reator* reator, CanalSimulado* canal) : canal_(canal), socket_(socket), reator_(reator) {};

const Instrumentacao& CamadaEnlace::instrumentacao() const {
    return instr_;
}

Instrumentacao& CamadaEnlace::instrumentacao() {
    return instr_;
}

void CamadaEnlace::enviar(const std::vector<uint8_t>& payload, const Endereco& destino) {
    std::vector<uint8_t> buf(TAM_LINK_HEADER + payload.size());
    memcpy(buf.data() + TAM_LINK_HEADER, payload.data(), payload.size());

    instr_.incrementar("quadros_enviados");
    instr_.incrementar("bytes_cabecalho_enviados", TAM_LINK_HEADER);
    instr_.incrementar("bytes_payload_enviados", static_cast<Instrumentacao::valor_t>(payload.size()));

    uint32_t checksum = crc32(buf);
    LinkHeader h;
    h.checksum = checksum;
    serializar_link_header(h, buf);
    if (canal_ != nullptr) {
        auto result = canal_->aplicar(buf);
        if (result.descartar) {
            instr_.incrementar("quadros_descartados_canal");
            return;
        }
        buf = result.quadro;
        Endereco e = destino;
        if (result.atraso_ms == 0) socket_->enviar(buf, destino);
        else {
            reator_->agendar(result.atraso_ms, [&, buf, destino, socket_ = socket_]() {
                socket_->enviar(buf, destino);
            });
        }
    } else {
        socket_->enviar(buf, destino);
    }
    return;
}

void CamadaEnlace::receber(const std::vector<uint8_t>& pdu, const Endereco& origem) {
    if (!(pdu.size() >= TAM_LINK_HEADER)) {
        instr_.incrementar("quadros_descartados_sem_cabecalho");
        std::cerr << "Tamanho do quadro é menor do que o necessário." << std::endl;
        return;
    }
    instr_.incrementar("quadros_recebidos");
    instr_.incrementar("bytes_cabecalho_recebidos", TAM_LINK_HEADER);
    instr_.incrementar("bytes_payload_recebidos", static_cast<Instrumentacao::valor_t>(pdu.size() - TAM_LINK_HEADER));

    LinkHeader h = desserializar_link_header(pdu);
    std::vector<uint8_t> pdu_copia(pdu.size());
    memcpy(pdu_copia.data(), pdu.data(), pdu.size());
    escreve_u32_be(pdu_copia, 0, 0);
    uint32_t checksum = crc32(pdu_copia);
    if (checksum != h.checksum) {
        instr_.incrementar("quadros_descartados_checksum");
        std::cerr << "Quadro corrompido" << std::endl;
        return;
    }
    std::vector<uint8_t> pdu_copia2(pdu.size() - TAM_LINK_HEADER);
    memcpy(pdu_copia2.data(), pdu.data() + TAM_LINK_HEADER, pdu.size() - TAM_LINK_HEADER);
    if (acima != nullptr) {
        acima->receber(pdu_copia2, origem);
        instr_.incrementar("quadros_entregues_ao_acima");
    } else {
        instr_.incrementar("quadros_descartados_sem_acima");
        std::cerr << "Acima não está conectado" << std::endl;
    }
}