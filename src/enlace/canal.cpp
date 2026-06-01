#include "canal.hpp"
#include "quadro.hpp"
#include <random>
#include <optional>
#include <iostream>

CanalSimulado::CanalSimulado(
    double p_perda_,
    double p_corrupcao_,
    double p_atraso_,
    uint32_t atraso_min_ms_,
    uint32_t atraso_max_ms_,
    int seed
) : p_perda(p_perda_), p_corrupcao(p_corrupcao_), p_atraso(p_atraso_), atraso_min_ms(atraso_min_ms_), atraso_max_ms(atraso_max_ms_), gerador(seed) {
    std::cerr << "[canal] seed=" << seed << "\n";
}

const Instrumentacao& CanalSimulado::instrumentacao() const {
    return instr_;
}

Instrumentacao& CanalSimulado::instrumentacao() {
    return instr_;
}

DecisaoCanal CanalSimulado::aplicar(std::vector<uint8_t> quadro) {
    std::uniform_real_distribution<double> tome(0.0, 1.0);

    DecisaoCanal dc;
    dc.quadro = quadro;
    instr_.incrementar("quadros_enviados");
    double perda_sorteada = tome(gerador);
    if (perda_sorteada < p_perda) {
        if (verboso) std::cerr << "[canal] pacote perdido" << std::endl;
        instr_.incrementar("quadros_perdidos");
        dc.descartar = true;
        return dc;
    }
    double corromper_sorteada = tome(gerador);
    if (corromper_sorteada < p_corrupcao && quadro.size() > TAM_LINK_HEADER) {
        if (verboso) std::cerr << "[canal] pacote corrompido" << std::endl;
        std::uniform_int_distribution<size_t> indice_byte(TAM_LINK_HEADER, quadro.size() - 1);
        std::uniform_int_distribution<int>    indice_bit(0, 7);
        size_t byte = indice_byte(gerador);
        int bit = indice_bit(gerador);
        quadro[byte] ^= (1 << bit);
        instr_.incrementar("quadros_corrompidos");
    }
    double atraso_sorteado = tome(gerador);
    if (atraso_sorteado < p_atraso) {
        if (verboso) std::cerr << "[canal] pacote com atraso" << std::endl;
        std::uniform_int_distribution<uint32_t> atraso_ms_uniforme(atraso_min_ms, atraso_max_ms);
        uint32_t atraso_ms = atraso_ms_uniforme(gerador);
        dc.atraso_ms = atraso_ms;
        instr_.incrementar("quadros_atrasados");
    }

    return dc;
}
