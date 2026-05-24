#include "canal.hpp"
#include "quadro.hpp"
#include <random>
#include <optional>
#include <iostream>

CanalSimulado::CanalSimulado(double p_perda_, double p_corrupcao_, int seed) : p_perda(p_perda_), p_corrupcao(p_corrupcao_), gerador(seed) {
    std::cerr << "[canal] seed=" << seed << "\n";
}

std::optional<std::vector<uint8_t>> CanalSimulado::aplicar(std::vector<uint8_t> quadro) {
    std::uniform_real_distribution<double> tome(0.0, 1.0);

     double perda_sorteada = tome(gerador);
    if (perda_sorteada < p_perda) {
        if (verboso) std::cerr << "[canal] pacote perdido" << std::endl;
        return std::nullopt;
    }
    double corromper_sorteada = tome(gerador);
    if (corromper_sorteada < p_corrupcao && quadro.size() > TAM_LINK_HEADER) {
        if (verboso) std::cerr << "[canal] pacote corrompido" << std::endl;
        std::uniform_int_distribution<size_t> indice_byte(TAM_LINK_HEADER, quadro.size() - 1);
        std::uniform_int_distribution<int>    indice_bit(0, 7);
        size_t byte = indice_byte(gerador);
        int bit = indice_bit(gerador);
        quadro[byte] ^= (1 << bit);
    }

    return quadro;
}
