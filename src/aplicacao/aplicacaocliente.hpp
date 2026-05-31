#pragma once
#include "camada.hpp"
#include "mensagem.hpp"
#include <functional>
#include <string>
#include <vector>
 
enum class EstadoConexao {
    DESCONECTADO,
    AGUARDANDO_ACK,
    CONECTADO
};

class CamadaAplicacaoCliente : public Camada {
public:

    // Usados para serem funções genéricas do que fazer quando recebe uma mensagem de chat ou um evento.
    using CallbackChat = std::function<void(const std::string& sender, const std::string& msg)>;
    using CallbackEvento = std::function<void(const std::string& descricao)>;
 
    CamadaAplicacaoCliente(const std::string& username, const Endereco& servidor, CallbackChat on_chat, CallbackEvento on_evento);

    void conectar();
    void enviar_chat(const std::string& texto);
    void desconectar();

    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu,    const Endereco& origem)  override;
 
    EstadoConexao estado()     const { return estado; }
    uint16_t      session_id() const { return session_id; }
 
private:
    std::string   username;
    Endereco      servidor;
    uint16_t      session_id = 0;
    EstadoConexao estado     = EstadoConexao::DESCONECTADO;
 
    CallbackChat   on_chat;
    CallbackEvento on_evento;
 
    void tratar_connect_ack (const std::vector<uint8_t>& payload);
    void tratar_connect_deny(const AppHeader& h);
    void tratar_chat        (const AppHeader& h, const std::vector<uint8_t>& payload);
    void tratar_disconnect  (const AppHeader& h);
    void tratar_error       (const std::vector<uint8_t>& payload);
 
    void enviar_ao_servidor(TipoMensagem tipo, const std::vector<uint8_t>& payload);
};
