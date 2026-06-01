#include "reator.hpp"
#include "udp_socket.hpp"
#include "enlace.hpp"
#include "canal.hpp"
#include "rotas.hpp"
#include "rede.hpp"
#include "transporte.hpp"
#include "conexao.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct Config {
    int repeticoes = 5;
    int mensagens = 100;
    int payload_size = 100;
    int timeout_ms = 20000;
    int porta_fisica_base = 5101;
    int porta_transporte_base = 4101;
    int seed_base = 42;
    int atraso_min_ms = 10;
    int atraso_max_ms = 30;
    std::string saida = "experimentos/atraso.csv";
};

struct Resultado {
    double p_atraso = 0.0;
    int atraso_min_ms = 0;
    int atraso_max_ms = 0;
    int repeticao = 0;
    int mensagens_planejadas = 0;
    int mensagens_entregues = 0;
    bool erro_conexao = false;
    bool timeout = false;
    long long retransmissoes = 0;
    long long dados_enviados = 0;
    long long quadros_enviados = 0;
    long long quadros_atrasados = 0;
    long long pacotes_reencaminhados = 0;
    long long tempo_exec_ms = 0;
};

static void imprimir_uso(const char* bin) {
    std::cerr << "Uso: " << bin << " [--repeticoes N] [--mensagens N] [--payload N] "
              << "[--timeout-ms N] [--porta-fisica-base N] [--porta-transporte-base N] "
              << "[--seed-base N] [--atraso-min-ms N] [--atraso-max-ms N] [--saida CAMINHO]\n";
}

static bool ler_int(const std::string& arg, int* out) {
    try {
        *out = std::stoi(arg);
    } catch (...) {
        return false;
    }
    return true;
}

static bool parse_args(int argc, char** argv, Config* cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") return false;
        auto next = [&](int* out) -> bool {
            if (i + 1 >= argc) return false;
            return ler_int(argv[++i], out);
        };
        if (arg == "--repeticoes") {
            if (!next(&cfg->repeticoes)) return false;
        } else if (arg == "--mensagens") {
            if (!next(&cfg->mensagens)) return false;
        } else if (arg == "--payload") {
            if (!next(&cfg->payload_size)) return false;
        } else if (arg == "--timeout-ms") {
            if (!next(&cfg->timeout_ms)) return false;
        } else if (arg == "--porta-fisica-base") {
            if (!next(&cfg->porta_fisica_base)) return false;
        } else if (arg == "--porta-transporte-base") {
            if (!next(&cfg->porta_transporte_base)) return false;
        } else if (arg == "--seed-base") {
            if (!next(&cfg->seed_base)) return false;
        } else if (arg == "--atraso-min-ms") {
            if (!next(&cfg->atraso_min_ms)) return false;
        } else if (arg == "--atraso-max-ms") {
            if (!next(&cfg->atraso_max_ms)) return false;
        } else if (arg == "--saida") {
            if (i + 1 >= argc) return false;
            cfg->saida = argv[++i];
        } else {
            return false;
        }
    }
    return true;
}

static bool bind_or_error(UdpSocket& s, uint16_t porta) {
    if (!s.udpBind(porta)) {
        std::cerr << "erro: nao foi possivel bind na porta " << porta << "\n";
        return false;
    }
    return true;
}

static Resultado rodar_cenario(double p_atraso, int repeticao, const Config& cfg) {
    Reator r;

    const uint16_t porta1 = static_cast<uint16_t>(cfg.porta_fisica_base);
    const uint16_t porta2 = static_cast<uint16_t>(cfg.porta_fisica_base + 1);
    const uint16_t porta3 = static_cast<uint16_t>(cfg.porta_fisica_base + 2);

    const uint16_t porta_transp1 = static_cast<uint16_t>(cfg.porta_transporte_base);
    const uint16_t porta_transp3 = static_cast<uint16_t>(cfg.porta_transporte_base + 2);

    UdpSocket s1;
    UdpSocket s2;
    UdpSocket s3;
    if (!bind_or_error(s1, porta1) || !bind_or_error(s2, porta2) || !bind_or_error(s3, porta3)) {
        std::exit(1);
    }

    const int seed_base = cfg.seed_base + repeticao * 10;
    CanalSimulado c1(0.0, 0.0, p_atraso, static_cast<uint32_t>(cfg.atraso_min_ms),
                     static_cast<uint32_t>(cfg.atraso_max_ms), seed_base + 1);
    CanalSimulado c2(0.0, 0.0, p_atraso, static_cast<uint32_t>(cfg.atraso_min_ms),
                     static_cast<uint32_t>(cfg.atraso_max_ms), seed_base + 2);
    CanalSimulado c3(0.0, 0.0, p_atraso, static_cast<uint32_t>(cfg.atraso_min_ms),
                     static_cast<uint32_t>(cfg.atraso_max_ms), seed_base + 3);

    CamadaEnlace e1(&s1, &r, &c1);
    CamadaEnlace e2(&s2, &r, &c2);
    CamadaEnlace e3(&s3, &r, &c3);

    TabelaRotas t1;
    TabelaRotas t2;
    TabelaRotas t3;

    CamadaRede n1(1, &t1);
    CamadaRede n2(2, &t2);
    CamadaRede n3(3, &t3);

    CamadaTransporte tr1(porta_transp1, &r);
    CamadaTransporte tr3(porta_transp3, &r);

    e1.conectar_acima(&n1);
    n1.conectar_abaixo(&e1);
    n1.conectar_acima(&tr1);
    tr1.conectar_abaixo(&n1);

    e2.conectar_acima(&n2);
    n2.conectar_abaixo(&e2);

    e3.conectar_acima(&n3);
    n3.conectar_abaixo(&e3);
    n3.conectar_acima(&tr3);
    tr3.conectar_abaixo(&n3);

    t1.inserir(2, "127.0.0.1", porta2);
    t1.inserir(3, "127.0.0.1", porta2);
    t2.inserir(1, "127.0.0.1", porta1);
    t2.inserir(3, "127.0.0.1", porta3);
    t3.inserir(2, "127.0.0.1", porta2);
    t3.inserir(1, "127.0.0.1", porta2);

    r.registrar_fd(s1.sock(), [&]() {
        auto rx = s1.receber(0);
        if (rx) e1.receber(rx->bytes, rx->origem);
    });
    r.registrar_fd(s2.sock(), [&]() {
        auto rx = s2.receber(0);
        if (rx) e2.receber(rx->bytes, rx->origem);
    });
    r.registrar_fd(s3.sock(), [&]() {
        auto rx = s3.receber(0);
        if (rx) e3.receber(rx->bytes, rx->origem);
    });

    Endereco dest3;
    dest3.logico = 3;
    dest3.porta = porta_transp3;
    Conexao& cenvio = tr1.abrir(porta_transp1, dest3, [](const std::vector<uint8_t>&) {});

    Endereco dest1;
    dest1.logico = 1;
    dest1.porta = porta_transp1;
    int entregues = 0;
    Conexao& crecv = tr3.abrir(porta_transp3, dest1, [&](const std::vector<uint8_t>&) {
        entregues++;
        if (entregues >= cfg.mensagens) {
            r.parar();
        }
    });

    (void)crecv;

    bool erro = false;
    bool timeout = false;
    cenvio.gerenciar_erro([&]() {
        erro = true;
        r.parar();
    });

    std::vector<uint8_t> payload(static_cast<size_t>(cfg.payload_size), 0x2A);
    r.agendar(0, [&]() {
        for (int i = 0; i < cfg.mensagens; ++i) {
            cenvio.enviar(payload);
        }
    });

    r.agendar(static_cast<uint32_t>(cfg.timeout_ms), [&]() {
        timeout = true;
        r.parar();
    });

    auto t_inicio = std::chrono::steady_clock::now();
    r.executar();
    auto t_fim = std::chrono::steady_clock::now();

    Resultado res;
    res.p_atraso = p_atraso;
    res.atraso_min_ms = cfg.atraso_min_ms;
    res.atraso_max_ms = cfg.atraso_max_ms;
    res.repeticao = repeticao;
    res.mensagens_planejadas = cfg.mensagens;
    res.mensagens_entregues = entregues;
    res.erro_conexao = erro;
    res.timeout = timeout;
    res.retransmissoes = tr1.instrumentacao().obter_ou_zero("retransmissoes");
    res.dados_enviados = tr1.instrumentacao().obter_ou_zero("dados_enviados");
    res.quadros_enviados = c1.instrumentacao().obter_ou_zero("quadros_enviados")
        + c2.instrumentacao().obter_ou_zero("quadros_enviados")
        + c3.instrumentacao().obter_ou_zero("quadros_enviados");
    res.quadros_atrasados = c1.instrumentacao().obter_ou_zero("quadros_atrasados")
        + c2.instrumentacao().obter_ou_zero("quadros_atrasados")
        + c3.instrumentacao().obter_ou_zero("quadros_atrasados");
    res.pacotes_reencaminhados = n2.instrumentacao().obter_ou_zero("pacotes_reencaminhados");
    res.tempo_exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_fim - t_inicio).count();
    return res;
}

static void escrever_csv(const std::string& path, const std::vector<Resultado>& resultados) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "erro: nao foi possivel abrir " << path << " para escrita\n";
        std::exit(1);
    }

    out << "p_atraso,atraso_min_ms,atraso_max_ms,repeticao,mensagens_planejadas,mensagens_entregues,"
           "erro_conexao,timeout,retransmissoes,dados_enviados,quadros_enviados,quadros_atrasados,"
           "pacotes_reencaminhados,taxa_entrega,taxa_retransmissoes,taxa_atraso,latencia_media_ms,tempo_exec_ms\n";
    out << std::fixed << std::setprecision(4);
    for (const auto& r : resultados) {
        double taxa_entrega = r.mensagens_planejadas > 0
            ? static_cast<double>(r.mensagens_entregues) / r.mensagens_planejadas
            : 0.0;
        double taxa_retrans = r.mensagens_planejadas > 0
            ? static_cast<double>(r.retransmissoes) / r.mensagens_planejadas
            : 0.0;
        double taxa_atraso = r.quadros_enviados > 0
            ? static_cast<double>(r.quadros_atrasados) / r.quadros_enviados
            : 0.0;
        double latencia_media = r.mensagens_entregues > 0
            ? static_cast<double>(r.tempo_exec_ms) / r.mensagens_entregues
            : 0.0;
        out << r.p_atraso << ","
            << r.atraso_min_ms << ","
            << r.atraso_max_ms << ","
            << r.repeticao << ","
            << r.mensagens_planejadas << ","
            << r.mensagens_entregues << ","
            << (r.erro_conexao ? 1 : 0) << ","
            << (r.timeout ? 1 : 0) << ","
            << r.retransmissoes << ","
            << r.dados_enviados << ","
            << r.quadros_enviados << ","
            << r.quadros_atrasados << ","
            << r.pacotes_reencaminhados << ","
            << taxa_entrega << ","
            << taxa_retrans << ","
            << taxa_atraso << ","
            << latencia_media << ","
            << r.tempo_exec_ms
            << "\n";
    }
}

int main(int argc, char** argv) {
    Config cfg;
    if (!parse_args(argc, argv, &cfg)) {
        imprimir_uso(argv[0]);
        return 1;
    }

    std::vector<double> atrasos = {0.0, 1, 2, 3, 5};
    std::vector<Resultado> resultados;
    resultados.reserve(static_cast<size_t>(cfg.repeticoes) * atrasos.size());

    for (double p : atrasos) {
        for (int rep = 1; rep <= cfg.repeticoes; ++rep) {
            resultados.push_back(rodar_cenario(p, rep, cfg));
        }
    }

    escrever_csv(cfg.saida, resultados);
    return 0;
}
