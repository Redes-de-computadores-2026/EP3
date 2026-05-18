#include "../comum/camada.hpp"
#include "../socket/udp_socket.hpp"
#include <iostream>


int main() {
    UdpSocket a;  a.udpBind(5001);
    UdpSocket b;  b.udpBind(5002);

    Endereco para_b;
    para_b.ip_fisico    = "127.0.0.1";
    para_b.porta_fisica = 5002;

    a.enviar({'h','i'}, para_b);

    auto r = b.receber(1000);
    if (!r) { std::cerr << "timeout\n"; return 1; }
    std::cout << "recebi " << r->bytes.size() << " bytes de " << r->origem.ip_fisico << ":" << r->origem.porta_fisica << "\n";

    // testa timeout: nada vai chegar em 'a' nesse intervalo
    auto vazio = a.receber(200);
    std::cout << (vazio ? "VAZIO ERRADO\n" : "timeout OK\n");
    return 0;
}
