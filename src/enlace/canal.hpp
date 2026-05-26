#pragma once
#include <random>
#include <vector>
#include <optional>

struct DecisaoCanal {
    bool descartar = false;
    std::vector<uint8_t> quadro;
    uint32_t atraso_ms = 0;
};

class CanalSimulado {
    double p_perda = 0.0;
    double p_corrupcao = 0.0;
    double p_atraso = 0.0;
    uint32_t atraso_min_ms = 0;
    uint32_t atraso_max_ms = 0;
    std::mt19937 gerador;
    bool verboso = false;
public:

    CanalSimulado(double p_perda, double p_corrupcao, double p_atraso, uint32_t atraso_min_ms, uint32_t atraso_max_ms, int seed = 42);
    DecisaoCanal aplicar(std::vector<uint8_t> quadro);
};
