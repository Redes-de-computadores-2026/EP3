#include "../comum/buffer.hpp"
#include <vector>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::vector<uint8_t> buf(4);
    escreve_u32_be(buf, 0, 0x12345678);
    assert(buf[0]==0x12 && buf[1]==0x34 && buf[2]==0x56 && buf[3]==0x78);
    assert(le_u32_be(buf, 0) == 0x12345678);

    std::vector<uint8_t> buf16(2);
    escreve_u16_be(buf16, 0, 0xABCD);
    assert(buf16[0]==0xAB && buf16[1]==0xCD);
    assert(le_u16_be(buf16, 0) == 0xABCD);

    std::cout << "buffer: 4 vetores OK\n";
}
