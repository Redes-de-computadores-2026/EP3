#include "aplicacao.hpp"
#include "mensagem.hpp"
#include <ctime>
#include <iostream>

CamadaAplicacao::CamadaAplicacao(CamadaTransporte& transporte, uint16_t porta_local, std::ostream& saida):
    transporte(transporte),
    porta_local(porta_local),
    session_id(static_cast<uint32_t>(std::time(nullptr))),
    saida(saida)
{}

void CamadaAplicacao::conectar(uint16_t no_remoto, uint16_t porta_remota) {
    Endereco destino;
    destino.logico = no_remoto;
    destino.porta  = porta_remota;
 
    conn = &transporte.abrir(
        porta_local,
        destino,
        [this](const std::vector<uint8_t>& bytes) { ao_receber(bytes); }
    );

    conn->gerenciar_erro([this]() {
        saida << "[APP] OUTRO nao respondeu — conexao encerrada\n";
    });

    AppHeader h{ CONNECT, session_id };
    conn->enviar(serializar_aplicacao(h));
 
    saida << "[APP] conectando ao no " << no_remoto << " (porta " << porta_remota << ")...\n";
}

void CamadaAplicacao::enviar_texto(const std::string& texto) {
    if (!conn) {
        saida << "[APP] nao conectado\n";
        return;
    }
    AppHeader h{ CHAT, session_id };
    conn->enviar(serializar_aplicacao(h, texto)); // Comunica à camada de transporte
}
 
void CamadaAplicacao::fechar() {
    if (!conn) return;
    AppHeader h{ DISCONNECT, session_id };
    conn->enviar(serializar_aplicacao(h)); // Comunica à camada de transporte
    sessao_aberta = false;
}

void CamadaAplicacao::ao_receber(const std::vector<uint8_t>& bytes) {
    if (bytes.size() < TAM_APP_HEADER) {
        std::cerr << "[APP] mensagem curta demais, ignorando\n";
        return;
    }
 
    AppHeader h = desserializar_header(bytes);
    std::string text = desserializar_texto(bytes);
 
    switch (static_cast<TipoMensagem>(h.msg_type)) {
        case CONNECT:
            sessao_aberta = true;
            saida << "[APP] sessao aberta (OUTRO session_id=" << h.session_id << ")\n";
            break;
 
        case CHAT:
            saida << "[OUTRO]: " << text << "\n";
            break;
 
        case DISCONNECT:
            sessao_aberta = false;
            saida << "[APP] OUTRO encerrou a sessao\n";
            break;
 
        default:
            std::cerr << "[APP] msg_type desconhecido: " << static_cast<int>(h.msg_type) << "\n";
    }
}
