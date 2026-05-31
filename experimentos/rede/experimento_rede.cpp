#include "experimento_rede.hpp"

#include "enlace.hpp"
#include "reator.hpp"
#include "rede.hpp"
#include "rotas.hpp"
#include "udp_socket.hpp"

#include <array>
#include <memory>
#include <sstream>
#include <vector>


class ColetorEntrega final : public Camada {
public:
    ColetorEntrega(Reator* reator, size_t alvo) {
        reator_ = reator;
        alvo_ = alvo;
    }

    void enviar(const std::vector<uint8_t>&, const Endereco&) override {}

    void receber(const std::vector<uint8_t>&, const Endereco&) override {
        ++recebidas_;
        if (recebidas_ >= alvo_) {
            reator_->parar();
        }
    }

    size_t recebidas() const {
        return recebidas_;
    }

private:
    Reator* reator_ = nullptr;
    size_t alvo_ = 0;
    size_t recebidas_ = 0;
};

struct NoExperimento {
    UdpSocket socket;
    std::unique_ptr<CanalSimulado> canal;
    std::unique_ptr<CamadaEnlace> enlace;
    std::unique_ptr<CamadaRede> rede;
    TabelaRotas rotas;
};

std::vector<uint8_t> montar_payload(uint32_t indice) {
    std::string texto = "msg-" + std::to_string(indice);
    return std::vector<uint8_t>(texto.begin(), texto.end());
}

void registrar_socket(Reator& reator, UdpSocket& socket, CamadaEnlace& enlace) {
    reator.registrar_fd(socket.sock(), [&]() {
        while (true) {
            auto recebido = socket.receber(0);
            if (!recebido) {
                break;
            }
            enlace.receber(recebido->bytes, recebido->origem);
        }
    });
}

Instrumentacao::valor_t ler(const std::map<std::string, Instrumentacao::valor_t>& metricas, const std::string& chave) {
    auto it = metricas.find(chave);
    if (it == metricas.end()) {
        return 0;
    }
    return it->second;
}

  
ResultadoExperimentoRede executar_experimento_rede(const ConfiguracaoExperimentoRede& config, uint32_t indice_cenario) {
    Reator reator;

    const uint16_t porta_1 = static_cast<uint16_t>(config.porta_base + indice_cenario * 10 + 1);
    const uint16_t porta_2 = static_cast<uint16_t>(config.porta_base + indice_cenario * 10 + 2);
    const uint16_t porta_3 = static_cast<uint16_t>(config.porta_base + indice_cenario * 10 + 3);

    NoExperimento no1;
    NoExperimento no2;
    NoExperimento no3;

    no1.socket.udpBind(porta_1);
    no2.socket.udpBind(porta_2);
    no3.socket.udpBind(porta_3);

    no1.canal = std::make_unique<CanalSimulado>(config.p_perda, config.p_corrupcao, config.p_atraso, config.atraso_min_ms, config.atraso_max_ms, static_cast<int>(config.seed + 1));
    no2.canal = std::make_unique<CanalSimulado>(config.p_perda, config.p_corrupcao, config.p_atraso, config.atraso_min_ms, config.atraso_max_ms, static_cast<int>(config.seed + 2));
    no3.canal = std::make_unique<CanalSimulado>(config.p_perda, config.p_corrupcao, config.p_atraso, config.atraso_min_ms, config.atraso_max_ms, static_cast<int>(config.seed + 3));

    no1.enlace = std::make_unique<CamadaEnlace>(&no1.socket, &reator, no1.canal.get());
    no2.enlace = std::make_unique<CamadaEnlace>(&no2.socket, &reator, no2.canal.get());
    no3.enlace = std::make_unique<CamadaEnlace>(&no3.socket, &reator, no3.canal.get());

    no1.rotas.inserir(3, "127.0.0.1", porta_2);
    no2.rotas.inserir(1, "127.0.0.1", porta_1);
    no2.rotas.inserir(3, "127.0.0.1", porta_3);
    no3.rotas.inserir(1, "127.0.0.1", porta_2);

    no1.rede = std::make_unique<CamadaRede>(1, &no1.rotas);
    no2.rede = std::make_unique<CamadaRede>(2, &no2.rotas);
    no3.rede = std::make_unique<CamadaRede>(3, &no3.rotas);

    ColetorEntrega coletor(&reator, config.mensagens);

    no1.enlace->conectar_acima(no1.rede.get());
    no1.rede->conectar_abaixo(no1.enlace.get());

    no2.enlace->conectar_acima(no2.rede.get());
    no2.rede->conectar_abaixo(no2.enlace.get());

    no3.enlace->conectar_acima(no3.rede.get());
    no3.rede->conectar_abaixo(no3.enlace.get());
    no3.rede->conectar_acima(&coletor);

    registrar_socket(reator, no1.socket, *no1.enlace);
    registrar_socket(reator, no2.socket, *no2.enlace);
    registrar_socket(reator, no3.socket, *no3.enlace);

    Endereco destino;
    destino.logico = 3;

    for (uint32_t i = 0; i < config.mensagens; ++i) {
        const uint32_t atraso_envio = i * config.intervalo_ms;
        reator.agendar(atraso_envio, [&, i]() {
            auto payload = montar_payload(i);
            no1.rede->enviar(payload, destino);
        });
    }

    const uint32_t limite = config.timeout_ms != 0
        ? config.timeout_ms
        : (config.mensagens * config.intervalo_ms) + config.atraso_max_ms + 1000;

    reator.agendar(limite, [&]() {
        reator.parar();
    });

    reator.executar();

    ResultadoExperimentoRede resultado;
    resultado.mensagens_enviadas = config.mensagens;
    resultado.mensagens_entregues = coletor.recebidas();
    resultado.rede1 = no1.rede->instrumentacao().exportar();
    resultado.rede2 = no2.rede->instrumentacao().exportar();
    resultado.rede3 = no3.rede->instrumentacao().exportar();
    return resultado;
}

std::string cabecalho_csv() {
    return "tipo,cenario,valor,p_perda,p_corrupcao,p_atraso,atraso_min_ms,atraso_max_ms,seed,mensagens,entregues,rede1_pacotes_enviados,rede1_pacotes_descartados_sem_rota,rede1_pacotes_descartados_destino_local,rede2_pacotes_recebidos,rede2_pacotes_reencaminhados,rede2_pacotes_descartados_ttl,rede2_pacotes_descartados_sem_rota,rede3_pacotes_recebidos,rede3_pacotes_entregues_ao_acima";
}

std::string linha_csv(const ConfiguracaoExperimentoRede& config, const ResultadoExperimentoRede& resultado, uint32_t indice_cenario, const std::string& tipo, double valor) {
    std::ostringstream out;
    out << tipo << ','
        << indice_cenario << ','
        << valor << ','
        << config.p_perda << ','
        << config.p_corrupcao << ','
        << config.p_atraso << ','
        << config.atraso_min_ms << ','
        << config.atraso_max_ms << ','
        << config.seed << ','
        << resultado.mensagens_enviadas << ','
        << resultado.mensagens_entregues << ','
        << ler(resultado.rede1, "pacotes_enviados") << ','
        << ler(resultado.rede1, "pacotes_descartados_sem_rota") << ','
        << ler(resultado.rede1, "pacotes_descartados_destino_local") << ','
        << ler(resultado.rede2, "pacotes_recebidos") << ','
        << ler(resultado.rede2, "pacotes_reencaminhados") << ','
        << ler(resultado.rede2, "pacotes_descartados_ttl") << ','
        << ler(resultado.rede2, "pacotes_descartados_sem_rota") << ','
        << ler(resultado.rede3, "pacotes_recebidos") << ','
        << ler(resultado.rede3, "pacotes_entregues_ao_acima");
    return out.str();
}
