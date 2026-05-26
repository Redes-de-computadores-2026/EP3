#include "reator.hpp"
#include <chrono>
#include <iostream>

int main() {
    Reator r;
    auto inicio = std::chrono::steady_clock::now();
    r.agendar(100, [&](){
      auto delta =  std::chrono::steady_clock::now() - inicio;
      std::cout << "tick em " << std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() << "ms" << std::endl;
  });
  r.agendar(200, [&](){
      std::cout << "tack\n";
      r.parar();   // termina o loop
  });
  r.executar();
    return 0;
}
