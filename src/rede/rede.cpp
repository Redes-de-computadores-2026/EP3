#include "rede.hpp"
#include <cstdint>
#include <vector>

CamadaRede::CamadaRede(uint16_t logico, TabelaRotas* rotas) {
  meu_logico = logico;
  rotas_ = rotas;
}

//O NetEncapsulate da issue
void CamadaRede::enviar(const std::vector<uint8_t>& payload, const Endereco& destino) {
  NetworkHeader cabecalho;
  cabecalho.logico_origem = meu_logico;
  cabecalho.logico_destino = destino.logico;
  cabecalho.protocolo = 0; //vai ser usado para multiplexação

  std::vector<uint8_t> pacote_rede(TAM_NETWORK_HEADER);

  if (destino.logico == meu_logico) {
    //Nao faz sentido enviar para mim mesmo, entao descartamos o pacote
    return;
  }

  serializar_network_header(cabecalho, pacote_rede);
  pacote_rede.insert(pacote_rede.end(), payload.begin(), payload.end());

  /*
    TO DO: IMPLEMENTAR METODO PARA SABER PARA QUAL NO PASSAR O PACOTE DEVE SER ENVIADO, BASEADO NO DESTINO LOGICO E NA TABELA DE ROTEAMENTO
  */

  if (abaixo) {
    abaixo->enviar(pacote_rede, destino);
  }
}

void CamadaRede::receber(const std::vector<uint8_t>& pdu, const Endereco& origem) {
  if (pdu.size() < TAM_NETWORK_HEADER) {
    //nao da para ter o conteudo do cabecalho, entao descartamos o pacote
    return;
  }

  NetworkHeader cabecalho = desserializar_network_header(pdu);
  
  if (cabecalho.logico_destino != meu_logico) {
    std::vector<uint8_t> pdu_copia = pdu;
    pdu_copia[4] = cabecalho.ttl - 1; //Decrementar o TTL
    if (pdu_copia[4] == 0) {
      //descartar o pacote
      return;
    }

    //Aqui cuidamos que o pacote nao morra e cuidamos que o TTL seja decrementado, mas nao sabemos para onde enviar, entao vamos reencaminhar o pacote para o proximo no, que vai cuidar disso

    abaixo->enviar(pdu_copia, origem); //Reencaminhar o pacote para o proximo no
    return;
  }

  std::vector<uint8_t> payload(pdu.begin() + TAM_NETWORK_HEADER, pdu.end());

  if (acima) {
    acima->receber(payload, origem);
  }
}

