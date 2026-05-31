#pragma once

#include "instrumentacao/instrumentacao.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

struct ConfiguracaoExperimentoRede {
    uint32_t mensagens = 20;
    uint32_t intervalo_ms = 20;
    uint32_t timeout_ms = 2000;
    uint32_t seed = 42;
    double p_perda = 0.0;
    double p_corrupcao = 0.0;
    double p_atraso = 0.0;
    uint32_t atraso_min_ms = 0;
    uint32_t atraso_max_ms = 0;
    uint16_t porta_base = 5100;
};

struct ResultadoExperimentoRede {
    size_t mensagens_enviadas = 0;
    size_t mensagens_entregues = 0;
    std::map<std::string, Instrumentacao::valor_t> rede1;
    std::map<std::string, Instrumentacao::valor_t> rede2;
    std::map<std::string, Instrumentacao::valor_t> rede3;
};

ResultadoExperimentoRede executar_experimento_rede(const ConfiguracaoExperimentoRede& config, uint32_t indice_cenario);

std::string cabecalho_csv();
std::string linha_csv(const ConfiguracaoExperimentoRede& config, const ResultadoExperimentoRede& resultado, uint32_t indice_cenario, const std::string& tipo, double valor);
