#include "experimento_rede.hpp"

#include <fstream>
#include <iostream>
#include <vector>

int main() {
    
    std::cout << cabecalho_csv() << '\n';

    const std::vector<double> perdas = {0.0, 0.05, 0.10, 0.20, 0.50};
    const std::vector<uint32_t> atrasos = {0, 10, 25, 50, 100};

    uint32_t indice = 0;

    for (double perda : perdas) {
        ConfiguracaoExperimentoRede config;
        config.p_perda = perda;
        config.seed = 42;
        config.mensagens = 20;
        config.intervalo_ms = 20;
        config.timeout_ms = 3000;
        auto resultado = executar_experimento_rede(config, indice);
        std::cout << linha_csv(config, resultado, indice, "perda", perda) << '\n';
        ++indice;
    }

    for (uint32_t atraso : atrasos) {
        ConfiguracaoExperimentoRede config;
        config.p_atraso = 1.0;
        config.atraso_min_ms = atraso;
        config.atraso_max_ms = atraso;
        config.seed = 100 + atraso;
        config.mensagens = 20;
        config.intervalo_ms = 20;
        config.timeout_ms = 3000 + atraso;
        auto resultado = executar_experimento_rede(config, indice);
        std::cout << linha_csv(config, resultado, indice, "atraso", static_cast<double>(atraso)) << '\n';
        ++indice;
    }

    return 0;
}
