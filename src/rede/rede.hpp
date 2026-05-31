#pragma once
#include "endereco.hpp"
#include "buffer.hpp"
#include "camada.hpp"

// Usar como base o código do https://github.com/torvalds/linux/blob/master/net/ipv4/ip_output.c
struct NetworkHeader {
    uint16_t logico_origem = 0;
    uint16_t logico_destino = 0;
    uint8_t ttl = 64;      // Valor padrao
    uint8_t protocolo = 0; // vai ser usado para multiplexação
};

constexpr size_t TAM_NETWORK_HEADER = 6;

inline void serializar_network_header(const NetworkHeader& h, std::vector<uint8_t>& buf) {
  escreve_u16_be(buf, 0, h.logico_origem);
  escreve_u16_be(buf, 2, h.logico_destino);
  buf[4] = h.ttl;
  buf[5] = h.protocolo;
}

inline NetworkHeader desserializar_network_header(const std::vector<uint8_t>& buf) {
  NetworkHeader h;
  h.logico_origem = le_u16_be(buf, 0);
  h.logico_destino = le_u16_be(buf, 2);
  h.ttl = buf[4];
  h.protocolo = buf[5];
  return h;
}

class CamadaRede : public Camada {
private:
  uint16_t meu_logico;
public:
  CamadaRede(uint16_t logico);
  ~CamadaRede() override = default;

  void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
  void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;
};
