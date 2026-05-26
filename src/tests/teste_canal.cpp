#include "canal.hpp"
#include <cassert>
#include <iostream>
#include <cstdint>

int main() {
    CanalSimulado canal(0.5, 0.0, 42);

    int count_null = 0;
    std::vector<uint8_t> tome = {0,0,0,0, 'x', 'y'};
    for (int i = 0; i < 1000; i++) {
        auto r = canal.aplicar({0,0,0,0, 'x', 'y'});
        if (!r.has_value()) count_null++;
    }

    std::cerr << "Perda observada: " << count_null << std::endl;
    assert(count_null > 450 && count_null < 550);

    int count_corrompidos = 0;
    CanalSimulado canal2(0.0, 0.5, 42);
    for (int i = 0; i < 1000; i++) {
        auto r = canal2.aplicar({0,0,0,0, 'x', 'y'});
        if (r.has_value() && r.value() != tome) count_corrompidos++;
    }
    std::cerr << "Corrompido observada: " << count_corrompidos << std::endl;
    assert(count_corrompidos > 450 && count_corrompidos < 550);


    CanalSimulado canal90(0.0, 1, 90);
    CanalSimulado canal91(0.0, 1, 90);
    auto r90 = canal90.aplicar({0,0,0,0, 'x', 'y', 'z'});
    auto r91 = canal91.aplicar({0,0,0,0, 'x', 'y', 'z'});
    for (int i = 0; i < 50; i++) {
        assert(r90.value() == r91.value());
        r90 = canal90.aplicar(r90.value());
        r91 = canal91.aplicar(r91.value());   
    }

    std::cout << "canal: 3 testes OK" << std::endl;
    return 0;
}