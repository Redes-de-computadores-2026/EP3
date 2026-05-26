#pragma once
#include "buffer.hpp"
#include <cstdint>
#include <vector>

struct LinkHeader {
    uint32_t checksum = 0;
};

constexpr size_t TAM_LINK_HEADER = 4;

inline void serializar_link_header(const LinkHeader& h, std::vector<uint8_t>& buf) {
    escreve_u32_be(buf, 0, h.checksum);
}

inline LinkHeader desserializar_link_header(const std::vector<uint8_t>& buf) {
    LinkHeader h;
    h.checksum = le_u32_be(buf, 0);
    return h;
}
