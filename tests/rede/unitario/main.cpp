#include <iostream>
#include <vector>
#include <cassert>
#include "rotas.hpp"
#include "rede.hpp"

using namespace std;

//somente para teste
class MockCamada : public Camada {
public:
    vector<uint8_t> payload;
    Endereco destino;
    
    vector<uint8_t> pdu;
    Endereco origem;
    
    int envios = 0;
    int recebimentos = 0;

    void enviar(const vector<uint8_t>& payload, const Endereco& destino) override {
        this->payload = payload;
        this->destino = destino;
        envios++;
    }

    void receber(const vector<uint8_t>& pdu, const Endereco& origem) override {
        this->pdu = pdu;
        this->origem = origem;
        recebimentos++;
    }
};

void teste_de_encapsulamento() {
    TabelaRotas tabela;
    tabela.inserir(20, "127.0.0.1", 5020);
    tabela.inserir(30, "127.0.0.1", 5030);
    CamadaRede rede(10, &tabela);
    MockCamada enlace;
    rede.conectar_abaixo(&enlace);

    vector<uint8_t> payload = {1, 2, 3};
    Endereco destino;
    destino.logico = 20;

    rede.enviar(payload, destino);

    assert(enlace.envios == 1);
    assert(enlace.payload.size() == 6 + payload.size());
    
    vector<uint8_t> pdu = enlace.payload;
    assert(pdu[0] == 0 && pdu[1] == 10); //resultado do shift e do & com 0xFF do logico_origem
    assert(pdu[2] == 0 && pdu[3] == 20); //Mesma coisa (20 é o destno)
    assert(pdu[4] == 64); //TTL padrao
    assert(pdu[5] == 0); //Protocolo padrao
    assert(pdu[6] == 1 && pdu[7] == 2 && pdu[8] == 3);
    
    cout << "teste_de_encapsulamento" << endl;
}

void teste_enviar_para_si_mesmo() {
    TabelaRotas tabela;
    tabela.inserir(20, "127.0.0.1", 5020);
    tabela.inserir(30, "127.0.0.1", 5030);
    CamadaRede rede(10, &tabela);
    MockCamada enlace;
    rede.conectar_abaixo(&enlace);

    vector<uint8_t> payload = {1};
    Endereco destino;
    destino.logico = 10;

    rede.enviar(payload, destino);

    assert(enlace.envios == 0);
    cout << "teste_enviar_loopback" << endl;
}

void teste_desacoplamento() {
    TabelaRotas tabela;
    tabela.inserir(20, "127.0.0.1", 5020);
    tabela.inserir(30, "127.0.0.1", 5030);
    CamadaRede rede(10, &tabela);
    MockCamada transporte;
    rede.conectar_acima(&transporte);
    vector<uint8_t> pdu = {0, 20, 0, 10, 64, 0, 1, 2};
    Endereco origem;
    
    rede.receber(pdu, origem);

    assert(transporte.recebimentos == 1);
    assert(transporte.pdu.size() == 2);
    assert(transporte.pdu[0] == 1);
    assert(transporte.pdu[1] == 2);
    
    cout << "teste_desacoplamento" << endl;
}

void teste_reencaminhar() {
    TabelaRotas tabela;
    tabela.inserir(20, "127.0.0.1", 5020);
    tabela.inserir(30, "127.0.0.1", 5030);
    CamadaRede rede(10, &tabela);
    MockCamada enlace;
    rede.conectar_abaixo(&enlace);
    uint8_t dest = 30;  // Endereco de destino (não sou eu)
                                       //Nao sou eu
    vector<uint8_t> pdu = {0, 20, 0, dest, 64, 0, 1, 2};
    Endereco origem;
    
    rede.receber(pdu, origem);

    assert(enlace.envios == 1);
    vector<uint8_t> pdu_forward = enlace.payload;
    assert(pdu_forward.size() == 8);
    assert(pdu_forward[4] == 63); //TTL decrementado
    
    cout << "teste_receber_reencaminhar" << endl;
}

void teste_receber_ttl_expirado() {
    TabelaRotas tabela;
    tabela.inserir(20, "127.0.0.1", 5020);
    tabela.inserir(30, "127.0.0.1", 5030);
    CamadaRede rede(10, &tabela);
    MockCamada enlace;
    rede.conectar_abaixo(&enlace);

    vector<uint8_t> pdu = {0, 20, 0, 30, 1, 0, 1, 2};
    Endereco origem;
    
    rede.receber(pdu, origem);

    assert(enlace.envios == 0);
    
    cout << "teste_receber_ttl_expirado" << endl;
}

int main() {
    cout << "Iniciando testes da Camada de Rede..." << endl;
    teste_de_encapsulamento();
    teste_enviar_para_si_mesmo();
    teste_desacoplamento();
    teste_reencaminhar();
    teste_receber_ttl_expirado();
    cout << "Todos os testes concluídos com sucesso!" << endl;
    return 0;
}