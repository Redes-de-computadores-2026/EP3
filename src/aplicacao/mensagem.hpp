#pragma once
#include "buffer.hpp"
#include <cstdint>
#include <vector>

enum class TipoMensagem : uint8_t
{
    CONNECT = 0x01,
    CONNECT_ACK = 0x02,
    CONNECT_DENY = 0x03,
    DISCONNECT = 0x04,
    CHAT = 0x05,
    ARQUIVO = 0x06,
    ERROR = 0x07
}

constexpr size_t TAM_APP_HEADER = 42;

struct AppHeader {
    uint8_t msg_type = 0; 
    uint16_t session_id = 0;
    uint8_t sender_len = 0;
    char sender[32] = {};
    uint16_t payload_length = 0;
    uint32_t timestamp = 0;
};

inline void serializar_app_header(const AppHeader& h, std::vector<uint8_t>& buf) {
    buf[0] = h.msg_type;
    escreve_u16_be(buf, 1, h.session_id);
    buf[3] = h.sender_len;
    std::memcpy(buf.data() + 4, h.sender, 32);
    escreve_u16_be(buf, 36, h.payload_length);
    escreve_u32_be(buf, 38, h.timestamp);
}

inline AppHeader desserializar_app_header(const std::vector<uint8_t>& buf) {
    AppHeader h;
    h.msg_type       = buf[0];
    h.session_id     = le_u16_be(buf,  1);
    h.sender_len     = buf[3];
    std::memcpy(h.sender, buf.data() + 4, 32);
    h.payload_length = le_u16_be(buf, 36);
    h.timestamp      = le_u32_be(buf, 38);
    return h;
}
