#include "mensagem.hpp"
#include <cassert>
#include <iostream>
 
void teste_roundtrip_msg() {
    AppHeader h{ MSG, 0xCAFEBABE };
    auto buf = montar_mensagem(h, "oi mundo");
 
    assert(buf.size() == TAM_APP_HEADER + 8);

    assert(buf[0] == MSG);

    assert(buf[1] == 0xCA);
    assert(buf[2] == 0xFE);
    assert(buf[3] == 0xBA);
    assert(buf[4] == 0xBE);

    AppHeader h2 = ler_header(buf);
    assert(h2.msg_type   == MSG);
    assert(h2.session_id == 0xCAFEBABE);
    assert(ler_texto(buf) == "oi mundo");
 
    std::cout << "[OK] MSG com texto\n";
}
 
void teste_hello_sem_texto() {
    AppHeader h{ HELLO, 1 };
    auto buf = montar_mensagem(h);
 
    assert(buf.size() == TAM_APP_HEADER);
 
    AppHeader h2 = ler_header(buf);
    assert(h2.msg_type   == HELLO);
    assert(h2.session_id == 1);
    assert(ler_texto(buf) == "");
 
    std::cout << "[OK] HELLO sem texto\n";
}
 
void teste_bye_sem_texto() {
    AppHeader h{ BYE, 42 };
    auto buf = montar_mensagem(h);
 
    assert(buf.size() == TAM_APP_HEADER);
    assert(ler_header(buf).msg_type   == BYE);
    assert(ler_header(buf).session_id == 42);
 
    std::cout << "[OK] BYE sem texto\n";
}
 
void teste_texto_vazio_explicito() {
    auto buf = montar_mensagem(AppHeader{ MSG, 0 }, "");
    assert(buf.size()    == TAM_APP_HEADER);
    assert(ler_texto(buf) == "");
 
    std::cout << "[OK] MSG com texto vazio explicito\n";
}
 
void teste_texto_com_espacos_e_especiais() {
    std::string texto = "ola, mundo! 123 @#$";
    auto buf = montar_mensagem(AppHeader{ MSG, 999 }, texto);
    assert(ler_texto(buf) == texto);
 
    std::cout << "[OK] MSG com texto especial: \"" << texto << "\"\n";
}
 
void teste_session_id_zero() {
    AppHeader h{ MSG, 0 };
    auto buf = montar_mensagem(h, "teste");
    assert(ler_header(buf).session_id == 0);
 
    std::cout << "[OK] session_id zero\n";
}
 
int main() {
    std::cout << "=== teste_mensagem ===\n";
    teste_roundtrip_msg();
    teste_hello_sem_texto();
    teste_bye_sem_texto();
    teste_texto_vazio_explicito();
    teste_texto_com_espacos_e_especiais();
    teste_session_id_zero();
    std::cout << "Todos os testes passaram.\n";
    return 0;
}
