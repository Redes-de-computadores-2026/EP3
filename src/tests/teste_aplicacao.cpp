#include "aplicacao.hpp"
#include "transporte.hpp"
#include "rede.hpp"
#include "reator.hpp"
#include "canal.hpp"
#include "enlace.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

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

    reator.registrar_fd(sa.sock(), [&](){ auto r = sa.receber(0); if (r) en1.receber(r->bytes, r->origem); });
    reator.registrar_fd(sb.sock(), [&](){ auto r = sb.receber(0); if (r) en2.receber(r->bytes, r->origem); });

    TabelaRotas tr1;
    TabelaRotas tr2;
    tr1.inserir(2, "127.0.0.1", 5002);
    tr2.inserir(1, "127.0.0.1", 5001);

    CamadaRede rede1(1, &tr1);
    CamadaRede rede2(2, &tr2);

    CamadaTransporte t1(7000, &reator);
    CamadaTransporte t2(7000, &reator);

    t1.conectar_abaixo(&rede1);
    rede1.conectar_abaixo(&en1);
    en1.conectar_acima(&rede1);
    rede1.conectar_acima(&t1);

    t2.conectar_abaixo(&rede2);
    rede2.conectar_abaixo(&en2);
    en2.conectar_acima(&rede2);
    rede2.conectar_acima(&t2);

    std::ostringstream out1, out2;
    CamadaAplicacao cam1(t1, 7000, out1);
    CamadaAplicacao cam2(t2, 7000, out2);
    
    cam1.conectar(2, 7000);
    cam2.conectar(1, 7000);

    reator.agendar(50,  [&](){ cam1.enviar_texto("Salve cachorro"); });
    reator.agendar(80,  [&](){ cam2.enviar_texto("OBA"); });
    reator.agendar(400, [&](){ reator.parar(); });

    reator.executar();

    assert(out2.str().find("Salve cachorro") != std::string::npos);
    assert(out1.str().find("OBA") != std::string::npos);
    std::cout << "chat OK\n";
    cam1.fechar();
    cam2.fechar();

    return 0;
}