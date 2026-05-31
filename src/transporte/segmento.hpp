#pragma once
#include "buffer.hpp"
#include <cstdint>
#include <vector>

constexpr size_t TAM_TRANSPORT_HEADER = 13;
enum FlagTransporte : uint8_t { DATA = 0x01, ACK = 0x02, SYN = 0x04, FIN = 0x08 };

struct TransportHeader {
    uint16_t porta_origem   = 0;
    uint16_t porta_destino  = 0;
    uint32_t seq_num        = 0;
    uint32_t ack_num        = 0;
    uint8_t  flags          = 0;
};

// O buffer tem que ter tamanho maior ou igual a TAM_TRANSPORT_HEADER
inline void serializar_transport_header(const TransportHeader& h, std::vector<uint8_t>& buf) {
    escreve_u16_be(buf, 0, h.porta_origem);
    escreve_u16_be(buf, 2, h.porta_destino);
    escreve_u32_be(buf, 4, h.seq_num);
    escreve_u32_be(buf, 8, h.ack_num);
    buf[12] = h.flags;
}

inline TransportHeader desserializar_transport_header(const std::vector<uint8_t>& buf) {
    TransportHeader h;
    h.porta_origem = le_u16_be(buf, 0);
    h.porta_destino = le_u16_be(buf, 2);
    h.seq_num = le_u32_be(buf, 4);
    h.ack_num = le_u32_be(buf, 8);
    h.flags = buf[12];
    return h;
}
