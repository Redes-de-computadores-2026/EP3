#include "aplicacaoservidor.hpp"
#include <cstring>
#include <iostream>

using namespace std;

void CamadaAplicacaoServidor::enviar(const vector<uint8_t>& payload, const Endereco& destino)
{
    if(abaixo == nullptr)
    {
        cerr << "[SERVIDOR] servidor não está conectado." << endl;
        return;
    }

    abaixo->enviar(payload, destino);
}

void CamadaAplicacaoServidor::receber(const vector<uint8_t>& pdu, const Endereco& origem)
{
    if(pdu.size() < TAM_APP_HEADER)
    {
        cerr << "[SERVIDOR] PDU menor que o mínimo necessário." << endl;
        return;
    }

    AppHeader h = desserializar_app_header(pdu);
    vector<uint8_t> payload (pdu.begin() + TAM_APP_HEADER, pdu.end());

    switch(static_cast<TipoMensagem> h.msg_type)
    {
        case TipoMensagem::CONNECT: tratar_connect(h, origem); break;
        case TipoMensagem::DISCONNECT: tratar_disconnect(h, origem); break;
        case TipoMensagem::CHAT: tratar_chat(h, payload, origem); break;
        default:
            cerr << "[SERVIDOR] Não trata-se esse tipo." << endl; 
    }
}

void CamadaAplicacaoServidor::tratar_connect(const AppHeader& h, const Endereco& origem)
{
    string nome = sender_do_header(h);
    uint16_t session_id_gerado = proximo_session_id;

    if(!verificar_nome_valido(nome))
    {
        cerr << "[SERVIDOR] Nome já utilizado." << endl;
        enviar_direto(TipoMensagem::CONNECT_DENY, session_id_gerado, "SERVIDOR", {})
        return;
    }

    proximo_session_id++;
    sessoes[session_id_gerado] = origem;
    usernames[session_id_gerado] = nome;

    cout << "[SERVIDOR] '" << nome << "' conectado com a sessão: " << session_id_gerado << endl;

    vector<uint8_t> ack(2);
    ack[0] = (session_id_gerado >> 8) & 0xFF;
    ack[1] =  session_id_gerado       & 0xFF;

    enviar_para(TipoMensagem::CONNECT_ACK, session_id_gerado, "SERVIDOR", ack);

    // broadcast avisando que "nome" conectou.
    for(auto& [id, _] : sessoes)
    {
        if (id == session_id_gerado) continue;
        enviar_para(TipoMensagem::CONNECTED, id, nome, {});
    }
}

// Impede nomes repetidos
bool verificar_nome_valido(string nome)
{
    for (auto& [_, n] : usernames)
    {
        if (nome == n)
            return false;
    }
    return true;
}

void CamadaAplicacaoServidor::tratar_disconnect(const AppHeader& h)
{
    uint16_t session_id = h.session_id;
    string nome = usernames[session_id];
    sessoes.erase(session_id);
    usernames.erase(session_id);

    cerr << "[SERVIDOR] " << nome << " desconectou." << endl;

    for (auto& [id, _] : sessoes)
    {
        enviar_para(TipoMensagem::DISCONNECT, id, nome, {});
    }
}

void CamadaAplicacaoServidor::tratar_chat(const AppHeader& h, const vector<uint8_t>& payload)
{
    string sender = sender_do_header(h);

    cerr << "[SERVIDOR] o usuário " << sender << "mandou a seguinte mensagem: " payload << endl;

    for (auto& [id, _] : sessoes)
    {
        if (id == session_id) continue;
        enviar_para(TipoMensagem::CHAT, id, sender, payload);
    }
}

void CamadaAplicacaoServidor::enviar_para(
    TipoMensagem               tipo,
    uint16_t                   session_dest,
    const string&         sender,
    const vector<uint8_t>& payload)
{
    if (!sessoes_.count(session_dest)) {
        cerr << "[SERVIDOR] sessão " << session_dest << " não encontrada\n";
        return;
    }
 
    AppHeader h = montar_header(tipo, session_dest, sender,
                                static_cast<uint16_t>(payload.size()));
 
    vector<uint8_t> buf(TAM_APP_HEADER + payload.size());
    memcpy(buf.data() + TAM_APP_HEADER, payload.data(), payload.size());
    serializar_app_header(h, buf);
 
    abaixo->enviar(buf, sessoes_[session_dest]);
}

void CamadaAplicacaoServidor::enviar_direto(
    TipoMensagem tipo,
    const Endereco& destino,
    const string& sender,
    const vector<uint8_t>& payload)
{
    AppHeader h = montar_header(tipo, 0, sender,
                                static_cast<uint16_t>(payload.size()));
    vector<uint8_t> buf(TAM_APP_HEADER + payload.size());
    memcpy(buf.data() + TAM_APP_HEADER, payload.data(), payload.size());
    serializar_app_header(h, buf);
    abaixo->enviar(buf, destino);
}