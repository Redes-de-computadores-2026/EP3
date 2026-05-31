#include "rotas.hpp"
#include "endereco.hpp"
#include <optional>

void TabelaRotas::inserir(uint16_t logico, const std::string& ip, uint16_t porta) {
    Endereco e;
    e.ip_fisico = ip;
    e.porta_fisica = porta;
    rotas_[logico] = e;
}

std::optional<Endereco> TabelaRotas::consultar(uint16_t logico) const {
    auto it = rotas_.find(logico);
    if (it == rotas_.end()) return std::nullopt;
    return it->second;
}
