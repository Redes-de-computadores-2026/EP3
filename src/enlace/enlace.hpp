#pragma once
#include "camada.hpp"
#include "udp_socket.hpp"
#include <vector>
#include <cstdint>

class CamadaEnlace : public Camada {
public:
    explicit CamadaEnlace(UdpSocket* socket);

    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;

private:
    UdpSocket* socket_;
};
