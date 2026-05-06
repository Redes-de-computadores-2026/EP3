#pragma once

#include "endereco.hpp"
#include <string>
#include <vector>

class Camada {
public:
  void enviar(const std::vector<uint8_t>& payload, const Endereco& destino);
  void receber(std::string pdu, const Endereco& origem);

  virtual ~Camada() = default;
protected:
  Camada* acima = nullptr;
  Camada* abaixo = nullptr;

  void conectar_acima(Camada* c) {
    acima = c;
  };
  void conectar_abaixo(Camada* c) {
    abaixo = c;
  }
};
