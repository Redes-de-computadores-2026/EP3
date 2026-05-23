#include "buffer.hpp"
#include <arpa/inet.h>
#include <cstring>

void escreve_u16_be(std::vector<uint8_t>& buf, size_t offset, uint16_t valor) {
    uint16_t valor_be = htons(valor);
    memcpy(buf.data() + offset, &valor_be, 2);
    
    return;
}

void escreve_u32_be(std::vector<uint8_t>& buf, size_t offset, uint32_t valor) {
    uint32_t valor_be = htonl(valor);
    memcpy(buf.data() + offset, &valor_be, 4);
    
    return;
}

uint16_t le_u16_be(const std::vector<uint8_t>& buf, size_t offset) {
    uint16_t valor_be;
    memcpy(&valor_be, buf.data() + offset, 2);
    return ntohs(valor_be);
}

uint32_t le_u32_be(const std::vector<uint8_t>& buf, size_t offset) {
    uint32_t valor_be;
    memcpy(&valor_be, buf.data() + offset, 4);
    return ntohl(valor_be);
}
