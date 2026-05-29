#pragma once
#include "buffer.hpp"
#include <cstdint>
#include <vector>

enum class TipoMensagem : uint8_t
{
    CONNECT = 0x01,
    CONNECT_ACK = 0x02,
    CONNECT_DENY = 0x03,
    CONNECTED = 0x04,
    DISCONNECT = 0x05,
    CHAT = 0x06,
    ARQUIVO = 0x07,
    ERROR = 0x08
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

inline AppHeader montar_header(
    TipoMensagem tipo,
    uint16_t     session_id,
    const std::string& sender,
    uint16_t     payload_length)
{
    AppHeader h;
    h.msg_type       = static_cast<uint8_t>(tipo);
    h.session_id     = session_id;
    h.payload_length = payload_length;
    h.timestamp      = static_cast<uint32_t>(std::time(nullptr));
 
    uint8_t len = static_cast<uint8_t>(std::min(sender.size(), size_t(31)));
    h.sender_len = len;
    std::memcpy(h.sender, sender.data(), len);
    h.sender[len] = '\0';
 
    return h;
}

inline std::string sender_do_header(const AppHeader& h) {
    return std::string(h.sender, h.sender_len);
}

