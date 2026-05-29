#pragma once
#include "camada.hpp"
#include <cstdint>

class CamadaTransporte : public Camada {
public:
    explicit CamadaTransporte(uint16_t porta_local);
    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;

private:
    uint16_t porta_local_;
};
