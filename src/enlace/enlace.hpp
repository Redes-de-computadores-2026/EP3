#pragma once
#include "camada.hpp"
#include "canal.hpp"
#include "udp_socket.hpp"
#include "reator.hpp"
#include "instrumentacao/instrumentacao.hpp"
#include <vector>
#include <cstdint>

class CamadaEnlace : public Camada {
    CanalSimulado* canal_ = nullptr;
    Instrumentacao instr_;
public:
    CamadaEnlace(UdpSocket* socket, Reator* reator) : CamadaEnlace(socket, reator, nullptr) {}
    CamadaEnlace(UdpSocket* socket, Reator* reator, CanalSimulado* canal);

    const Instrumentacao& instrumentacao() const;
    Instrumentacao& instrumentacao();

    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;

private:
    UdpSocket* socket_;
    Reator* reator_;
};
