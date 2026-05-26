#include "checksum.hpp"
#include <array>
#include <cstddef>

static std::array<uint32_t, 256> gerar_tabela() {
    std::array<uint32_t, 256> tabela{};
    uint32_t c;
    for (uint32_t i = 0; i < 256; i++) {
        c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) c = (c >> 1) ^ 0xEDB88320;
            else c = c >> 1;
        }
        tabela[i] = c;
    }

    return tabela;
}

uint32_t crc32(const uint8_t* dados, size_t len) {
    static const auto tabela = gerar_tabela();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; ++i) {
        uint8_t indice = (crc ^ dados[i]) & 0xFF;
        crc = tabela[indice] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

uint32_t crc32(const std::vector<uint8_t>& dados) {
    return crc32(dados.data(), dados.size());
}

static const auto tabela = gerar_tabela();