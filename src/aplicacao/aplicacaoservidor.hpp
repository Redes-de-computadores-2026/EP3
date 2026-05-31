#pragma once
#include "camada.hpp"
#include "mensagem.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>
 
class CamadaAplicacaoServidor : public Camada {
public:
    CamadaAplicacaoServidor() = default;
 
    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem)     override;
 
private:
    uint16_t proximo_session_id = 1;
 
    std::map<uint16_t, Endereco>    sessoes;
    std::map<uint16_t, std::string> usernames;
 
    void tratar_connect   (const AppHeader& h, const Endereco& origem);
    void tratar_disconnect(const AppHeader& h);
    void tratar_chat      (const AppHeader& h, const std::vector<uint8_t>& payload);
 
    void enviar_direto(TipoMensagem tipo, const Endereco& destino, const std::string& sender, const std::vector<uint8_t>& payload)

    void enviar_para(TipoMensagem tipo, uint16_t session_dest, const std::string& sender, const std::vector<uint8_t>& payload);
};
