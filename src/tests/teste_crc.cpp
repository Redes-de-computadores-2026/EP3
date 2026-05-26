#include "../enlace/checksum.hpp"
#include <cassert>
#include <iostream>
#include <string>

static std::vector<uint8_t> str(const std::string &s)
{
    return std::vector<uint8_t>(s.begin(), s.end());
}

int main()
{
    assert(crc32(str("")) == 0x00000000);
    assert(crc32(str("a")) == 0xE8B7BE43);
    assert(crc32(str("abc")) == 0x352441C2);
    assert(crc32(str("123456789")) == 0xCBF43926);
    assert(crc32(str("The quick brown fox jumps over the lazy dog")) == 0x414FA339);
    std::cout << "CRC32: 5 vetores OK\n";
    return 0;
}