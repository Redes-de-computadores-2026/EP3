#pragma once
#include "../comum/endereco.hpp"
#include "../comum/camada.hpp"


//Usar como base o código do https://github.com/torvalds/linux/blob/master/net/ipv4/ip_output.c
// 
struct CabecalhoRede {
    uint16_t logico_origem = 0;
    uint16_t logico_destino = 0;
    uint8_t ttl = 64; //Vamor padrao
    uint8_t protocolo = 0; //vai ser usado para multiplexação
};

class CamadaRede : public Camada {
private:
  uint16_t meu_logico;
public:
  CamadaRede(uint16_t logico);
  ~CamadaRede() override = default;

  void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
  void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;
};