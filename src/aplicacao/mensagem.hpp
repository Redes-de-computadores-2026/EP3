#pragma once
#include "buffer.hpp"
#include <cstdint>
#include <vector>
#include <string>

enum TipoMensagem : uint8_t {
    CONNECT = 0x01,
    CHAT   = 0x02,
    DISCONNECT   = 0x03,
};


constexpr size_t TAM_APP_HEADER = 5;

struct AppHeader {
    uint8_t msg_type = 0; 
    uint32_t session_id = 0;
};

inline std::vector<uint8_t> serializar_aplicacao(const AppHeader& h, const std::string& texto = "")
{
    std::vector<uint8_t> buf(TAM_APP_HEADER + texto.size());
    buf[0] = h.msg_type;
    escreve_u32_be(buf, 1, h.session_id);
    std::copy(texto.begin(), texto.end(), buf.begin() + TAM_APP_HEADER);
    return buf;
}

inline AppHeader desserializar_header(const std::vector<uint8_t>& buf) {
    AppHeader h;
    h.msg_type   = buf[0];
    h.session_id = le_u32_be(buf, 1);
    return h;
}
 
inline std::string desserializar_texto(const std::vector<uint8_t>& buf) {
    if (buf.size() <= TAM_APP_HEADER) return "";
    return std::string(buf.begin() + TAM_APP_HEADER, buf.end());
}


