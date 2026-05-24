#pragma once
#include <random>
#include <vector>
#include <optional>

class CanalSimulado {
    double p_perda = 0.0;
    double p_corrupcao = 0.0;
    std::mt19937 gerador;
    bool verboso = false;
public:

    CanalSimulado(double p_perda, double p_corrupcao, int seed = 42);
    std::optional<std::vector<uint8_t>> aplicar(std::vector<uint8_t> quadro);
};
