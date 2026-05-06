#include "camada_de_rede.hpp"

using namespace std;

//https://discourse.julialang.org/t/convert-uint16-to-two-uint8/7115
int serialization_16_into_8_le(vector<uint8_t> &buffer, uint16_t value) {
  buffer.push_back(value & 0xFF); //parte baixa
  buffer.push_back(value >> 8); //parte alta
  return 0;
}

int serialization_16_into_8_be(vector<uint8_t> &buffer, uint16_t value) {
  buffer.push_back(value >> 8); //parte alta
  buffer.push_back(value & 0xFF); //parte baixa
  return 0;
}

int remontando_cabecalho_rede_le(const vector<uint8_t>& buffer, CabecalhoRede &cabecalho) {
  //Desfazendo o que fizemos no serialization_16_into_8_le
  cabecalho.logico_origem = buffer[0] | (buffer[1] << 8);
  cabecalho.logico_destino = buffer[2] | (buffer[3] << 8);
  cabecalho.ttl = buffer[4];
  cabecalho.protocolo = buffer[5];
  return 0;
}


int remontando_cabecalho_rede_be(const vector<uint8_t>& buffer, CabecalhoRede &cabecalho) {
  //Desfazendo o que fizemos no serialization_16_into_8_le
  cabecalho.logico_origem = (buffer[0] << 8) | buffer[1];
  cabecalho.logico_destino = (buffer[2] << 8) | buffer[3];
  cabecalho.ttl = buffer[4];
  cabecalho.protocolo = buffer[5];
  return 0;
}

CamadaRede::CamadaRede(uint16_t logico) {
  meu_logico = logico;
}

//O NetEncapsulate da issue
void CamadaRede::enviar(const vector<uint8_t>& payload, const Endereco& destino) {
  CabecalhoRede cabecalho;
  cabecalho.logico_origem = meu_logico;
  cabecalho.logico_destino = destino.logico;
  cabecalho.protocolo = 0; //vai ser usado para multiplexação

  vector<uint8_t> pacote_rede;

  //verificação basica de envio
  if (destino.logico == meu_logico) {
    //Nao faz sentido enviar para mim mesmo, entao descartamos o pacote
    return;
  }

  /*
    TO DO: IMPLEMENTAR METODO PARA QUEBRAR EM PACOTES SE O PAYLOAD FOR MUITO GRANDE
  */

  //Serializar o cabecalho
  
  //Transformar tudo em uint8_t e colocar no pacote_rede
  //little endian -> para trocar so trocar le para be
  //Aqui é feito 2 pushs
  serialization_16_into_8_le(pacote_rede, cabecalho.logico_origem);
  serialization_16_into_8_le(pacote_rede, cabecalho.logico_destino);

  pacote_rede.push_back(cabecalho.ttl);
  pacote_rede.push_back(cabecalho.protocolo);

  //Anexar o payload
  pacote_rede.insert(pacote_rede.end(), payload.begin(), payload.end());

  /*
    TO DO: IMPLEMENTAR METODO PARA SABER PARA QUAL NO PASSAR O PACOTE DEVE SER ENVIADO, BASEADO NO DESTINO LOGICO E NA TABELA DE ROTEAMENTO
  */

  if (abaixo) {
    abaixo->enviar(pacote_rede, destino);
  }
}

void CamadaRede::receber(const vector<uint8_t>& pdu, const Endereco& origem) {
  //Como colocamos o cabeçalho na ordem, origem, destino, ttl, protocolo, precisamos ler nessa ordem
  // [origem, destino, ttl, protocolo, payload]
  if (pdu.size() < 6) {
    //nao da para ter o conteudo do cabecalho, entao descartamos o pacote
    return; 
  }

  CabecalhoRede cabecalho;
  //Remontar o cabecalho a partir do pdu recebido
  //little endian -> para trocar so trocar le para be
  //Pode nao ser necessario, mas é muito mais simples fazer verificação assim do que por index.
  remontando_cabecalho_rede_le(pdu , cabecalho);
  
  if (cabecalho.logico_destino != meu_logico) {
    //Nao é para mim
    vector<uint8_t> pdu_copia = pdu;
    pdu_copia[4] = cabecalho.ttl - 1; //Decrementar o TTL
    if (pdu_copia[4] == 0) {
      //descartar o pacote
      return;
    }

    //Aqui cuidamos que o pacote nao morra e cuidamos que o TTL seja decrementado, mas nao sabemos para onde enviar, entao vamos reencaminhar o pacote para o proximo no, que vai cuidar disso

    abaixo->enviar(pdu_copia, origem); //Reencaminhar o pacote para o proximo no
    return;
  }

  //Desanexar o cabeçalho do payload
  vector<uint8_t> payload(pdu.begin() + 6, pdu.end());

  if (acima) {
    acima->receber(payload, origem);
  }
}

