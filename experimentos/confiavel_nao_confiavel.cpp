#include "reator.hpp"
#include "udp_socket.hpp"
#include "enlace.hpp"
#include "canal.hpp"
#include "rotas.hpp"
#include "rede.hpp"
#include "transporte.hpp"
#include "conexao.hpp"
#include "aplicacao.hpp"
#include "mensagem.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Config {
    int      repeticoes            = 5;
    int      mensagens             = 100;
    int      payload_size          = 100;
    int      timeout_ms            = 15000;
    int      porta_fisica_base     = 5201;
    int      porta_transporte_base = 4201;
    int      seed_base             = 42;
    std::string saida              = "experimentos/confiavel.csv";
};

static void imprimir_uso(const char* bin) {
    std::cerr << "Uso: " << bin
              << " [--repeticoes N] [--mensagens N] [--payload N]"
                 " [--timeout-ms N] [--porta-fisica-base N]"
                 " [--porta-transporte-base N] [--seed-base N] [--saida CAMINHO]\n";
}

static bool ler_int(const std::string& s, int* out) {
    try { *out = std::stoi(s); } catch (...) { return false; }
    return true;
}

static bool parse_args(int argc, char** argv, Config* cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--help") return false;
        auto next = [&](int* out) -> bool {
            if (i + 1 >= argc) return false;
            return ler_int(argv[++i], out);
        };
        if      (a == "--repeticoes")            { if (!next(&cfg->repeticoes))            return false; }
        else if (a == "--mensagens")             { if (!next(&cfg->mensagens))             return false; }
        else if (a == "--payload")               { if (!next(&cfg->payload_size))          return false; }
        else if (a == "--timeout-ms")            { if (!next(&cfg->timeout_ms))            return false; }
        else if (a == "--porta-fisica-base")     { if (!next(&cfg->porta_fisica_base))     return false; }
        else if (a == "--porta-transporte-base") { if (!next(&cfg->porta_transporte_base)) return false; }
        else if (a == "--seed-base")             { if (!next(&cfg->seed_base))             return false; }
        else if (a == "--saida") {
            if (i + 1 >= argc) return false;
            cfg->saida = argv[++i];
        } else { return false; }
    }
    return true;
}

static bool bind_or_error(UdpSocket& s, uint16_t porta) {
    if (!s.udpBind(porta)) {
        return false;
    }
    return true;
}

struct Resultado {
    double p_perda             = 0.0;
    int    repeticao           = 0;
    bool   confiavel           = false;
    int    mensagens_planejadas = 0;
    int    mensagens_entregues  = 0;
    bool   erro_conexao        = false;
    bool   timeout             = false;
    long long retransmissoes   = 0;
    long long dados_enviados   = 0;
    long long quadros_enviados  = 0;
    long long quadros_perdidos  = 0;
    long long tempo_exec_ms    = 0;
};

static Resultado rodar_confiavel(double p_perda, int repeticao, const Config& cfg) {
    Reator r;

    const uint16_t porta1   = static_cast<uint16_t>(cfg.porta_fisica_base);
    const uint16_t porta2   = static_cast<uint16_t>(cfg.porta_fisica_base + 1);
    const uint16_t porta_t1 = static_cast<uint16_t>(cfg.porta_transporte_base);
    const uint16_t porta_t2 = static_cast<uint16_t>(cfg.porta_transporte_base + 1);

    UdpSocket s1, s2;
    if (!bind_or_error(s1, porta1) || !bind_or_error(s2, porta2)) std::exit(1);

    const int seed = cfg.seed_base + repeticao * 10;
    CanalSimulado c1(p_perda, 0.0, 0.0, 0, 0, seed + 1);
    CanalSimulado c2(p_perda, 0.0, 0.0, 0, 0, seed + 2);

    CamadaEnlace e1(&s1, &r, &c1);
    CamadaEnlace e2(&s2, &r, &c2);

    TabelaRotas t1, t2;
    t1.inserir(2, "127.0.0.1", porta2);
    t2.inserir(1, "127.0.0.1", porta1);

    CamadaRede n1(1, &t1);
    CamadaRede n2(2, &t2);

    CamadaTransporte tr1(porta_t1, &r);
    CamadaTransporte tr2(porta_t2, &r);

    e1.conectar_acima(&n1);  n1.conectar_abaixo(&e1);
    n1.conectar_acima(&tr1); tr1.conectar_abaixo(&n1);

    e2.conectar_acima(&n2);  n2.conectar_abaixo(&e2);
    n2.conectar_acima(&tr2); tr2.conectar_abaixo(&n2);

    r.registrar_fd(s1.sock(), [&]() {
        auto rx = s1.receber(0);
        if (rx) e1.receber(rx->bytes, rx->origem);
    });
    r.registrar_fd(s2.sock(), [&]() {
        auto rx = s2.receber(0);
        if (rx) e2.receber(rx->bytes, rx->origem);
    });

    int entregues = 0;
    Conexao& crecv = tr2.abrir(porta_t2, Endereco{1, porta_t1},
        [&](const std::vector<uint8_t>&) {
            if (++entregues >= cfg.mensagens) r.parar();
        });
    (void)crecv;

    std::ostringstream sink;
    CamadaAplicacao app1(tr1, porta_t1, sink);
    app1.conectar(2, porta_t2);

    bool timeout = false;
    std::string texto(static_cast<size_t>(cfg.payload_size), 'x');
    r.agendar(10, [&]() { // aguarda CONNECT chegar antes de enviar CHATs
        for (int i = 0; i < cfg.mensagens; ++i) {
            app1.enviar_texto(texto);
        }
    });
    r.agendar(static_cast<uint32_t>(cfg.timeout_ms), [&]() { timeout = true; r.parar(); });

    auto t0 = std::chrono::steady_clock::now();
    r.executar();
    auto t1e = std::chrono::steady_clock::now();

    Resultado res;
    res.p_perda             = p_perda;
    res.repeticao           = repeticao;
    res.confiavel           = true;
    res.mensagens_planejadas = cfg.mensagens;
    res.mensagens_entregues  = entregues;
    res.erro_conexao        = false;
    res.timeout             = timeout;
    res.retransmissoes      = tr1.instrumentacao().obter_ou_zero("retransmissoes");
    res.dados_enviados      = tr1.instrumentacao().obter_ou_zero("dados_enviados");
    res.quadros_enviados    = c1.instrumentacao().obter_ou_zero("quadros_enviados")
                            + c2.instrumentacao().obter_ou_zero("quadros_enviados");
    res.quadros_perdidos    = c1.instrumentacao().obter_ou_zero("quadros_perdidos")
                            + c2.instrumentacao().obter_ou_zero("quadros_perdidos");
    res.tempo_exec_ms       = std::chrono::duration_cast<std::chrono::milliseconds>(t1e - t0).count();
    return res;
}

class ReceptorBestEffort : public Camada {
public:
    std::function<void(const std::vector<uint8_t>&)> cb;
    void receber(const std::vector<uint8_t>& pdu, const Endereco&) override {
        if (cb) cb(pdu);
    }
    void enviar(const std::vector<uint8_t>&, const Endereco&) override {}
};

static Resultado rodar_best_effort(double p_perda, int repeticao, const Config& cfg) {
    Reator r;

    const uint16_t porta1 = static_cast<uint16_t>(cfg.porta_fisica_base + 10);
    const uint16_t porta2 = static_cast<uint16_t>(cfg.porta_fisica_base + 11);

    UdpSocket s1, s2;
    if (!bind_or_error(s1, porta1) || !bind_or_error(s2, porta2)) std::exit(1);

    const int seed = cfg.seed_base + repeticao * 10;
    CanalSimulado c1(p_perda, 0.0, 0.0, 0, 0, seed + 1);
    CanalSimulado c2(p_perda, 0.0, 0.0, 0, 0, seed + 2);

    CamadaEnlace e1(&s1, &r, &c1);
    CamadaEnlace e2(&s2, &r, &c2);

    TabelaRotas t1, t2;
    t1.inserir(2, "127.0.0.1", porta2);
    t2.inserir(1, "127.0.0.1", porta1);

    CamadaRede n1(1, &t1);
    CamadaRede n2(2, &t2);

    e1.conectar_acima(&n1); n1.conectar_abaixo(&e1);
    e2.conectar_acima(&n2); n2.conectar_abaixo(&e2);

    int entregues = 0;
    ReceptorBestEffort receptor;
    receptor.cb = [&](const std::vector<uint8_t>&) {
        if (++entregues >= cfg.mensagens) r.parar();
    };
    n2.conectar_acima(&receptor);

    r.registrar_fd(s1.sock(), [&]() {
        auto rx = s1.receber(0);
        if (rx) e1.receber(rx->bytes, rx->origem);
    });
    r.registrar_fd(s2.sock(), [&]() {
        auto rx = s2.receber(0);
        if (rx) e2.receber(rx->bytes, rx->origem);
    });

    AppHeader h{ CHAT, 0 };
    std::string texto(static_cast<size_t>(cfg.payload_size), 'x');
    std::vector<uint8_t> payload = serializar_aplicacao(h, texto);

    bool timeout = false;
    r.agendar(0, [&]() {
        Endereco dest{ 2, 0 };
        for (int i = 0; i < cfg.mensagens; ++i) {
            n1.enviar(payload, dest);
        }
    });
    r.agendar(static_cast<uint32_t>(cfg.timeout_ms), [&]() { timeout = true; r.parar(); });

    auto t0 = std::chrono::steady_clock::now();
    r.executar();
    auto t1e = std::chrono::steady_clock::now();

    Resultado res;
    res.p_perda             = p_perda;
    res.repeticao           = repeticao;
    res.confiavel           = false;
    res.mensagens_planejadas = cfg.mensagens;
    res.mensagens_entregues  = entregues;
    res.erro_conexao        = false;
    res.timeout             = timeout;
    res.retransmissoes      = 0;
    res.dados_enviados      = cfg.mensagens;
    res.quadros_enviados    = c1.instrumentacao().obter_ou_zero("quadros_enviados")
                            + c2.instrumentacao().obter_ou_zero("quadros_enviados");
    res.quadros_perdidos    = c1.instrumentacao().obter_ou_zero("quadros_perdidos")
                            + c2.instrumentacao().obter_ou_zero("quadros_perdidos");
    res.tempo_exec_ms       = std::chrono::duration_cast<std::chrono::milliseconds>(t1e - t0).count();
    return res;
}


static void escrever_csv(const std::string& path, const std::vector<Resultado>& resultados) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "erro: nao foi possivel abrir " << path << " para escrita\n";
        std::exit(1);
    }
    out << "p_perda,repeticao,confiavel,mensagens_planejadas,mensagens_entregues,"
           "erro_conexao,timeout,retransmissoes,dados_enviados,"
           "quadros_enviados,quadros_perdidos,"
           "taxa_entrega,taxa_retransmissoes,perda_quadros,tempo_exec_ms\n";
    out << std::fixed << std::setprecision(4);
    for (const auto& res : resultados) {
        double taxa_entrega  = res.mensagens_planejadas > 0
            ? static_cast<double>(res.mensagens_entregues) / res.mensagens_planejadas : 0.0;
        double taxa_retrans  = res.mensagens_planejadas > 0
            ? static_cast<double>(res.retransmissoes) / res.mensagens_planejadas : 0.0;
        double perda_quadros = res.quadros_enviados > 0
            ? static_cast<double>(res.quadros_perdidos) / res.quadros_enviados : 0.0;
        out << res.p_perda << ","
            << res.repeticao << ","
            << (res.confiavel ? 1 : 0) << ","
            << res.mensagens_planejadas << ","
            << res.mensagens_entregues << ","
            << (res.erro_conexao ? 1 : 0) << ","
            << (res.timeout ? 1 : 0) << ","
            << res.retransmissoes << ","
            << res.dados_enviados << ","
            << res.quadros_enviados << ","
            << res.quadros_perdidos << ","
            << taxa_entrega << ","
            << taxa_retrans << ","
            << perda_quadros << ","
            << res.tempo_exec_ms
            << "\n";
    }
}

int main(int argc, char** argv) {
    Config cfg;
    if (!parse_args(argc, argv, &cfg)) {
        imprimir_uso(argv[0]);
        return 1;
    }

    std::vector<double> perdas = {0.0, 0.05, 0.10, 0.20, 0.50};
    std::vector<Resultado> resultados;
    resultados.reserve(perdas.size() * static_cast<size_t>(cfg.repeticoes) * 2);

    for (double p : perdas) {
        for (int rep = 1; rep <= cfg.repeticoes; ++rep) {
            resultados.push_back(rodar_confiavel(p, rep, cfg));
            resultados.push_back(rodar_best_effort(p, rep, cfg));
        }
    }

    escrever_csv(cfg.saida, resultados);
    return 0;
}
