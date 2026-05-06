#pragma once

#include "endereco.hpp"
#include <vector>
#include <cstdint>

class Camada {
public:
  /// Caminho descendente da pilha. Chamada pela camada IMEDIATAMENTE
  /// ACIMA quando ela quer transmitir um PDU para um destino.
  ///
  /// Esta camada deve: encapsular (anexar seu header), opcionalmente
  /// transformar o destino (ex: lógico → físico), e delegar para
  /// `abaixo_->enviar(...)` (ou para o socket, se for a Enlace).
  ///
  /// @param payload  bytes da camada superior (já serializados).
  /// @param destino  para onde mandar; quais campos usar depende da camada.
  virtual void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) = 0;

  /// Caminho ascendente da pilha. Chamada pela camada IMEDIATAMENTE
  /// ABAIXO quando ela quer receber uma mensagem de uma origem.
  ///
  /// Esta camada deve: desencapsular (desanexar seu header), opcionalmente
  /// transformar a origem (ex: física → lógica), e delegar para
  /// `acima->receber(...)` (ou para o cliente, se for a Aplicação).
  ///
  /// @param pdu  bytes da camada inferior (já serializados).
  /// @param origem  de onde veio; quais campos usar depende da camada.
  virtual void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) = 0;

  void conectar_acima(Camada* c) {
    acima = c;
  };
  void conectar_abaixo(Camada* c) {
    abaixo = c;
  }

  virtual ~Camada() = default;
protected:
  Camada* acima = nullptr;
  Camada* abaixo = nullptr;
};
