#pragma once

#include <cstdint>
#include <string>

struct Endereco {
    uint16_t logico = 0;
    uint16_t porta = 0;
    std::string ip_fisico = "";
    uint16_t porta_fisica = 0;
};
