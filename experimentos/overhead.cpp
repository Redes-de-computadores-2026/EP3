#include "segmento.hpp" // TAM_TRANSPORT_HEADER

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

static constexpr int HEADER_ENLACE     = 4;   // CRC32
static constexpr int HEADER_REDE       = 6;   // TAM_NETWORK_HEADER
static constexpr int HEADER_TRANSPORTE = static_cast<int>(TAM_TRANSPORT_HEADER);
static constexpr int HEADER_APLICACAO  = 5;   // msg_type (1) + session_id (4)
static constexpr int HEADER_TOTAL      = HEADER_ENLACE + HEADER_REDE + HEADER_TRANSPORTE + HEADER_APLICACAO;

struct Config {
    std::string saida = "experimentos/overhead.csv";
};

static void imprimir_uso(const char* bin) {
    std::cerr << "Uso: " << bin << " [--saida CAMINHO]\n";
}

static bool parse_args(int argc, char** argv, Config* cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") return false;
        if (arg == "--saida") {
            if (i + 1 >= argc) return false;
            cfg->saida = argv[++i];
        } else {
            return false;
        }
    }
    return true;
}

struct Resultado {
    int payload_size = 0;
    int header_enlace = 0;
    int header_rede = 0;
    int header_transporte = 0;
    int header_aplicacao = 0;
    int header_total = 0;
    int bytes_totais = 0;
    double ratio_overhead = 0.0;   // header_total / payload
    double eficiencia = 0.0;       // payload / bytes_totais
};

static Resultado calcular(int payload_size) {
    Resultado r;
    r.payload_size       = payload_size;
    r.header_enlace      = HEADER_ENLACE;
    r.header_rede        = HEADER_REDE;
    r.header_transporte  = HEADER_TRANSPORTE;
    r.header_aplicacao   = HEADER_APLICACAO;
    r.header_total       = HEADER_TOTAL;
    r.bytes_totais       = HEADER_TOTAL + payload_size;
    r.ratio_overhead     = payload_size > 0
                               ? static_cast<double>(HEADER_TOTAL) / payload_size
                               : 0.0;
    r.eficiencia         = r.bytes_totais > 0
                               ? static_cast<double>(payload_size) / r.bytes_totais
                               : 0.0;
    return r;
}

static void escrever_csv(const std::string& path, const std::vector<Resultado>& resultados) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "erro: nao foi possivel abrir " << path << " para escrita\n";
        std::exit(1);
    }

    out << "payload_size,"
           "header_enlace,header_rede,header_transporte,header_aplicacao,header_total,"
           "bytes_totais,ratio_overhead,eficiencia\n";
    out << std::fixed << std::setprecision(4);
    for (const auto& r : resultados) {
        out << r.payload_size << ","
            << r.header_enlace << ","
            << r.header_rede << ","
            << r.header_transporte << ","
            << r.header_aplicacao << ","
            << r.header_total << ","
            << r.bytes_totais << ","
            << r.ratio_overhead << ","
            << r.eficiencia
            << "\n";
    }
}

int main(int argc, char** argv) {
    Config cfg;
    if (!parse_args(argc, argv, &cfg)) {
        imprimir_uso(argv[0]);
        return 1;
    }

    // Payloads representativos: de 1 byte até 1400 bytes (próximo do MTU Ethernet)
    std::vector<int> payloads = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 1400};

    std::vector<Resultado> resultados;
    resultados.reserve(payloads.size());
    for (int p : payloads) {
        resultados.push_back(calcular(p));
    }

    escrever_csv(cfg.saida, resultados);

    // Resumo no stderr para conferência rápida
    std::cerr << "overhead fixo por mensagem: " << HEADER_TOTAL << " bytes\n"
              << "  enlace:     " << HEADER_ENLACE     << " bytes\n"
              << "  rede:       " << HEADER_REDE        << " bytes\n"
              << "  transporte: " << HEADER_TRANSPORTE  << " bytes\n"
              << "  aplicacao:  " << HEADER_APLICACAO   << " bytes\n";

    return 0;
}
