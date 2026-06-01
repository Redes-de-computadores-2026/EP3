#pragma once
#include "camada.hpp"
#include "segmento.hpp"
#include "conexao.hpp"
#include "instrumentacao/instrumentacao.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <functional>

class CamadaTransporte : public Camada {
    std::map<ChaveConexao, Conexao> conexoes_;
    Reator* reator_;
    Instrumentacao instr_;
public:
    CamadaTransporte(uint16_t porta_local, Reator* r);
    Conexao& abrir(uint16_t porta_local, const Endereco& destino, std::function<void(const std::vector<uint8_t>&)> callback);
    const Instrumentacao& instrumentacao() const;
    Instrumentacao& instrumentacao();
    void enviar(const std::vector<uint8_t>& payload, const Endereco& destino) override;
    void receber(const std::vector<uint8_t>& pdu, const Endereco& origem) override;
    void _enviar_segmento(const ChaveConexao& chave, const std::vector<uint8_t>& payload,uint8_t flags, uint32_t seq, uint32_t ack);
private:
    uint16_t porta_local_;
    void registrar_envio_segmento(std::size_t payload_size, uint8_t flags);
};
