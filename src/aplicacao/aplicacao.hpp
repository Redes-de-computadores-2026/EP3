#pragma once
#include "transporte.hpp"
#include "endereco.hpp"
#include <cstdint>
#include <ostream>
#include <iostream>
#include <string>
#include <vector>

class CamadaAplicacao {
public:
    CamadaAplicacao(CamadaTransporte& transporte, uint16_t porta_local, std::ostream& saida = std::cout);
    void conectar(uint16_t no_remoto, uint16_t porta_remota);
    void enviar_texto(const std::string& texto);
    void fechar();
 
private:
    void ao_receber(const std::vector<uint8_t>& bytes);
    CamadaTransporte& transporte;
    Conexao*          conn = nullptr;
    uint16_t          porta_local;
    uint32_t          session_id;
    bool              sessao_aberta = false;
    std::ostream&     saida;
}