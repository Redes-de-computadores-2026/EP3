#include "segmento.hpp"
#include <vector>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::vector<uint8_t> buf(13);
    TransportHeader h;
    h.porta_origem = 0x1234;
    h.porta_destino = 0x5678;
    h.seq_num = 0xAABBCCDD;
    h.ack_num = 0x11223344;
    h.flags = DATA|ACK;
    serializar_transport_header(h, buf);
    auto h2 = desserializar_transport_header(buf);

    assert(h.porta_origem == h2.porta_origem);
    assert(h.porta_destino == h2.porta_destino);
    assert(h.seq_num == h2.seq_num);
    assert(h.ack_num == h2.ack_num);
    assert(h.flags == h2.flags);
    assert(buf[12] == 0x03);
    assert(buf[0] == 0x12 && buf[1] == 0x34);

    std::cout << "segmento: 5 campos OK\n";

}
