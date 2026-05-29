#include "app_cliente.hpp"
#include <cstring>
#include <iostream>

using namespace std;
 
CamadaAplicacaoCliente::CamadaAplicacaoCliente(const string& username, const Endereco& servidor, CallbackChat on_chat, CallbackEvento on_evento):
    username(username),
    servidor(servidor),
    on_chat(on_chat),
    on_evento(on_evento)
{}

void CamadaAplicacaoCliente::conectar()
{
    if (estado() != EstadoConexao::DESCONECTADO)
    {
        cerr << "[CLIENTE] Já conectado." << endl;
        return;
    }

    estado = EstadoConexao::AGUARDANDO_ACK;
    enviar_ao_servidor(TipoMensagem:CONNECT, {});
    cout << "[CLIENTE] Enviado pedido de conexão, esperando ACK." << endl;
}

void CamadaAplicacaoCliente::enviar_chat(const std::string& texto)
{
    if(estado() != EstadoConexao::CONECTADO)
    {
        cerr << "[CLIENTE] Não é possível mandar mensagens sem estar conectado." << endl;
        return;
    }

    std::vector<uint8_t> payload(texto.begin(), texto.end());
    enviar_ao_servidor(TipoMensagem::CHAT, payload);
}

void CamadaAplicacaoCliente::desconectar()
{
    if(estado() == EstadoConexao::DESCONECTADO) return;
    enviar_ao_servidor(TipoMensagem::DISCONNECT, {});
    session_id = 0;
    estado = EstadoConexao::DESCONECTADO;
    cout << "[CLIENTE] desconectado." << endl;
}

void CamadaAplicacaoCliente::enviar(const std::vector<uint8_t>& payload, const Endereco& destino)
{
    if (abaixo == nullptr) {
        cerr << "[CLIENTE] abaixo não conectado" << endl;
        return;
    }
    abaixo->enviar(payload, destino);
}

void CamadaAplicacaoCliente::receber(const std::vector<uint8_t>& pdu, const Endereco& origem)
{
    if (pdu.size() < TAM_APP_HEADER) {
        cerr << "[CLIENTE] PDU menor que o mínimo necessário." << endl;
        return;
    }
 
    AppHeader h = desserializar_app_header(pdu);
    std::vector<uint8_t> payload (pdu.begin() + TAM_APP_HEADER, pdu.end());

        switch (static_cast<TipoMensagem>(h.msg_type))
        {
            case TipoMensagem::CONNECT_ACK:  tratar_connect_ack (h, payload); break;
            case TipoMensagem::CONNECT_DENY: tratar_connect_deny(h);          break;
            case TipoMensagem::CHAT:         tratar_chat        (h, payload); break;
            case TipoMensagem::DISCONNECT:   tratar_disconnect  (h);          break;
            case TipoMensagem::ERROR:        tratar_error       (h, payload); break;
            default:
                std::cerr << "[CLIENTE] Não trata-se esse tipo." << endl;
    }
}

void CamadaAplicacaoCliente::tratar_connect_ack(const std::vector<uint8_t>& payload)
{
    if (payload.size() < 2) {
        std::cerr << "[CLIENTE] CONNECT_ACK sem session_id." << endl;
        return;
    }

    session_id = static_cast<uint16_t>((payload[0] << 8) | payload[1]); // pega o session id
    estado = EstadoConexao::CONECTADO;
    on_evento("Cliente de sessão: "+to_string(session_id)+" conectado!");
}

void CamadaAplicacaoCliente::tratar_connect_deny(const AppHeader& h)
{
    estado = EstadoConexao::DESCONECTADO;
    on_evento("Cliente foi impossibilitado de conectar");
}

void CamadaAplicacaoCliente::tratar_chat(const AppHeader& h, const std::vector<uint8_t>& payload)
{
    string nome_da_mensagem = sender_do_header(h);
    string texto(payload.begin(), payload.end());
    on_chat(nome_da_mensagem, texto);
}

void CamadaAplicacaoCliente::tratar_disconnect(const AppHeader& h)
{
    string nome_de_quem_saiu = sender_do_header(h);
    on_evento(nome_de_quem_saiu + "saiu.");
}

void CamadaAplicacaoCliente::tratar_error(const std::vector<uint8_t>& payload)
{
    string erro(payload.begin(), payload.end());
    on_evento("Erro: "+erro);
}

void CamadaAplicacaoCliente::enviar_ao_servidor(
    TipoMensagem               tipo,
    const std::vector<uint8_t>& payload)
{
    AppHeader h = montar_header(tipo, session_id_, username_,
                                static_cast<uint16_t>(payload.size()));
 
    std::vector<uint8_t> buf(TAM_APP_HEADER + payload.size());
    std::memcpy(buf.data() + TAM_APP_HEADER, payload.data(), payload.size());
    serializar_app_header(h, buf);
 
    abaixo->enviar(buf, servidor_);
}
