#pragma once
#include "camada.hpp"
#include "canal.hpp"
#include "udp_socket.hpp"
#include <vector>
#include <cstdint>

class CamadaEnlace : public Camada {
    CanalSimulado* canal_ = nullptr;
public:
    CamadaEnlace(UdpSocket* socket) : CamadaEnlace(socket, nullptr) {}
    CamadaEnlace(UdpSocket* socket, CanalSimulado* canal);

    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;

private:
    UdpSocket* socket_;
};
