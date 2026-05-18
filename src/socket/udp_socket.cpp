#include "udp_socket.hpp"
#include "endereco.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <optional>
#include <vector>
#include <cstdint>
#include <sys/select.h>

bool UdpSocket::udpBind(uint16_t port) {
    sockaddr_in eu = {};
    eu.sin_family = AF_INET;
    eu.sin_addr.s_addr = htonl(INADDR_ANY);
    eu.sin_port = htons(port);
    int c = bind(sock_, (sockaddr*)&eu, sizeof(eu));
    if (c == -1) return false;
    return true;
}

ssize_t UdpSocket::enviar(const std::vector<uint8_t>& buf, const Endereco& destino) {
    sockaddr_in eu = {};
    eu.sin_family = AF_INET;
    eu.sin_port = htons(destino.porta_fisica);
    if (inet_pton(AF_INET, destino.ip_fisico.c_str(), &eu.sin_addr) != 1) {
      std::cerr << "host inválido: " << destino.ip_fisico << std::endl;
      return -1;
    }

    // converte Endereco destino no sockaddr_in
    ssize_t t = sendto(sock_ , buf.data(), buf.size(), 0, (const sockaddr*)&eu, sizeof(eu));
    return t;
}

std::optional<Recebido> UdpSocket::receber(int timeout_ms) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sock_, &rfds);

    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int r = select(sock_ + 1, &rfds, nullptr, nullptr, &tv);
    if (r <= 0) return std::nullopt;   // timeout (0) ou erro (<0)

    std::vector<uint8_t> buf(65535);
    sockaddr_in origem = {};
    socklen_t origem_len = sizeof(origem);
    ssize_t t = recvfrom(sock_, buf.data(), 65535, 0, (sockaddr*)&origem, &origem_len);

    if (t < 0) return std::nullopt;

    buf.resize(t);
    char ip_buf[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &origem.sin_addr, ip_buf, sizeof(ip_buf));
    Endereco e;
    e.ip_fisico = ip_buf;
    e.porta_fisica = ntohs(origem.sin_port);
    Recebido recebido;
    recebido.origem = e;
    recebido.bytes = buf;
    return recebido;
}
