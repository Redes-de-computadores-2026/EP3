#include "rotas.hpp"
#include <cassert>
#include <iostream>

int main() {
    TabelaRotas t;
    t.inserir(1, "127.0.0.1", 5001);
    t.inserir(2, "10.0.0.5", 6000);

    auto r1 = t.consultar(1);
    assert(r1.has_value());
    assert(r1->ip_fisico == "127.0.0.1" && r1->porta_fisica == 5001);

    assert(!t.consultar(99).has_value());

    std::cout << "Teste de rotas OK!" << std::endl;

}
