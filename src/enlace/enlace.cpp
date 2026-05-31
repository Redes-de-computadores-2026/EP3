#include "enlace.hpp"
#include "quadro.hpp"
#include "canal.hpp"
#include "checksum.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstring>

CamadaEnlace::CamadaEnlace(UdpSocket* socket, Reator* reator, CanalSimulado* canal) : canal_(canal), socket_(socket), reator_(reator) {};

void CamadaEnlace::enviar(const std::vector<uint8_t>& payload, const Endereco& destino) {
    std::vector<uint8_t> buf(TAM_LINK_HEADER + payload.size());
    memcpy(buf.data() + TAM_LINK_HEADER, payload.data(), payload.size());
    uint32_t checksum = crc32(buf);
    LinkHeader h;
    h.checksum = checksum;
    serializar_link_header(h, buf);
    if (canal_ != nullptr) {
        auto result = canal_->aplicar(buf);
        if (result.descartar) return;
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
        std::cerr << "Tamanho do quadro é menor do que o necessário." << std::endl;
        return;
    }
    LinkHeader h = desserializar_link_header(pdu);
    std::vector<uint8_t> pdu_copia(pdu.size());
    memcpy(pdu_copia.data(), pdu.data(), pdu.size());
    escreve_u32_be(pdu_copia, 0, 0);
    uint32_t checksum = crc32(pdu_copia);
    if (checksum != h.checksum) {
        std::cerr << "Quadro corrompido" << std::endl;
        return;
    }
    std::vector<uint8_t> pdu_copia2(pdu.size() - TAM_LINK_HEADER);
    memcpy(pdu_copia2.data(), pdu.data() + TAM_LINK_HEADER, pdu.size() - TAM_LINK_HEADER);
    if (acima != nullptr) {
        acima->receber(pdu_copia2, origem);
    } else {
        std::cerr << "Acima não está conectado" << std::endl;
    }
}