#include "aplicacao.hpp"
#include "transporte.hpp"
#include "rede.hpp"
#include "reator.hpp"
#include "canal.hpp"
#include "enlace.hpp"

int main()
{  
    UdpSocket sa;
    UdpSocket sb;
    Reator reator;

    sa.udpBind(5001);
    sb.udpBind(5002);

    CanalSimulado cs(0.0, 0.0, 0.0, 0, 0);

    CamadaEnlace en1(&sa, &reator, &cs);
    CamadaEnlace en2(&sb, &reator, &cs);

    TabelaRotas t1;
    TabelaRotas t2;
    t1.inserir(2, "127.0.0.1", 5002);
    t2.inserir(1, "127.0.0.1", 5001);

    CamadaRede rede1(1, &t1);
    CamadaRede rede2(2, &t2);

    CamadaTransporte t1(100, &reator);
    CamadaTransporte t2(100, &reator);

    t1.conectar_abaixo(&rede1);
    rede1.conectar_abaixo(&en1);
    en1.conectar_acima(&rede1);
    rede1.conectar_acima(&t1);

    t2.conectar_abaixo(&rede2);
    rede2.conectar_abaixo(&en2);
    en2.conectar_acima(&rede2);
    rede2.conectar_acima(&t2);

    CamadaAplicacao cam1(t1, 7000, std::cout);
    CamadaAplicacao cam2(t2, 7000, std::cout);
    
    cam1.conectar(2, 7000);
    cam2.conectar(1, 7000);

    cam1.enviar_texto("Salve cachorro");
    cam2.enviar_texto("OBA");

    cam1.fechar();
    cam2.fechar();

    return 0;
}