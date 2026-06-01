# EP3
EP3 da disciplina de MAC0352 -- Redes de Computadores

### Como compilar:
Use os comandos definidos no Makefile.
make clean - execute sempre antes de compilar para limpar os executáveis anteriores
make all - gera o binário executável na pasta build.
make debug - compila com flags extras de proteção que pegam comandos de UB, out-of-bounds e use-after-free
make release - compila com flags de otimização que deixam o executável mais perfomático

### Como executar o chat:
O `make all` gera o binário `build/rede`. O chat roda com **dois nós**, um por terminal.

```
./build/rede <1|2> [perda]
```
- `<1|2>` — id do nó. Nó 1 usa UDP 5001, nó 2 usa UDP 5002; os dois conversam entre si.
- `[perda]` (opcional) — probabilidade de perda no canal simulado, de `0` a `1` (default `0`).

**Exemplo (dois terminais):**
```
# terminal 1
./build/rede 1

# terminal 2
./build/rede 2
```
Digite uma linha e tecle Enter para enviar; aparece como `[OUTRO]: ...` no outro terminal.
Digite `/sair` (ou Ctrl+D) para encerrar a sessão.

**Simulando um canal com perda** (mostra a retransmissão recuperando os pacotes):
```
# terminal 1
./build/rede 1 0.3

# terminal 2
./build/rede 2 0.3
```
Quanto maior a perda, mais retransmissões — em perda muito alta as conexões chegam a desistir.

> Suba os dois nós com poucos segundos de diferença: o nó que sobe primeiro fica
> retransmitindo o handshake até o outro aparecer, mas desiste depois de um tempo.

### Testes:
Cada camada tem alvos de teste no Makefile, por exemplo:
```
make teste_rede          # camada de rede + tabela de rotas
make teste_transporte    # stop-and-wait: ACK, retransmissão, dedup
make teste_rede_enlace   # integração rede + enlace sobre UDP real
```

### Estrutura do projeto:
EP3/
├── README.md             ← Guia de como executar, informações gerais
├── Makefile              
├── .gitignore            
├── src/                  ← Pasta onde ficam os códigos do projeto
│   ├── main_no.cpp       ← entry point: monta a pilha e roda um nó do chat
│   ├── comum/            ←
│   ├── enlace/           ←
│   ├── rede/             ←
│   ├── transporte/       ←
│   ├── aplicacao/        ←
│   └── socket/           ←
├── tests/                ←
├── experimentos/         ←
├── docs/                 ←
└── config/               ←

