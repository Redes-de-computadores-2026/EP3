#pragma once
#include "endereco.hpp"
#include <map>
#include <optional>
#include <cstdint>
#include <string>

class TabelaRotas {
    std::map<uint16_t, Endereco> rotas_;
public:
    void inserir(uint16_t logico, const std::string& ip, uint16_t porta);
    std::optional<Endereco> consultar(uint16_t logico) const;
};
