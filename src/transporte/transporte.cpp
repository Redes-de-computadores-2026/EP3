#include "transporte.hpp"
#include "segmento.hpp"
#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>

CamadaTransporte::CamadaTransporte(uint16_t porta_local) : porta_local_(porta_local) {}

void CamadaTransporte::enviar(const std::vector<uint8_t>& payload, const Endereco& destino) {
    TransportHeader h;
    h.porta_destino = destino.porta;
    h.porta_origem = porta_local_;
    h.flags = DATA;
    std::vector<uint8_t> buf(TAM_TRANSPORT_HEADER + payload.size());
    serializar_transport_header(h, buf);
    memcpy(buf.data() + TAM_TRANSPORT_HEADER, payload.data(), payload.size());
    if (abaixo) {
        abaixo->enviar(buf, destino);
    }    
}

void CamadaTransporte::receber(const std::vector<uint8_t>& pdu, const Endereco& origem) {
    if (!(pdu.size() >= TAM_TRANSPORT_HEADER)) {
        std::cerr << "Tamanho do segmento é menor do que o necessário." << std::endl;
        return;
    }
    TransportHeader h = desserializar_transport_header(pdu);
    std::vector<uint8_t> payload(pdu.size() - TAM_TRANSPORT_HEADER);
    memcpy(payload.data(), pdu.data() + TAM_TRANSPORT_HEADER, pdu.size() - TAM_TRANSPORT_HEADER);
    Endereco copia_origem = origem;
    copia_origem.porta = h.porta_origem;
    if (acima) {
        acima->receber(payload, copia_origem);
    }
}
