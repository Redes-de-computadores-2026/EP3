#pragma once
#include <sys/socket.h>
#include "endereco.hpp"
#include <unistd.h>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <optional>

struct Recebido {
    std::vector<uint8_t> bytes;
    Endereco origem;
};

class UdpSocket {
    int sock_;
public:
    UdpSocket() : sock_(socket(AF_INET, SOCK_DGRAM, 0)) {
        if (sock_ < 0) { throw std::runtime_error(std::string("socket() falhou: ") + strerror(errno)); }
        int yes = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    };

    ~UdpSocket() { close(sock_); };

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    bool udpBind(uint16_t port = 9000);
    ssize_t enviar(const std::vector<uint8_t>& buf, const Endereco& destino);
    std::optional<Recebido> receber(int timeout_ms);

    int sock() const { return sock_; };
};
