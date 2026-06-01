# EP3
EP3 da disciplina de MAC0352 -- Redes de Computadores

### Como compilar:
Use os comandos definidos no Makefile.
make clean - execute sempre antes de compilar para limpar os executáveis anteriores
make all - gera o binário executável na pasta build.
make debug - compila com flags extras de proteção que pegam comandos de UB, out-of-bounds e use-after-free
make release - compila com flags de otimização que deixam o executável mais perfomático

### Como executar:
`./build/main_no <id>`

### Estrutura do projeto:
EP3/
├── README.md             ← Guia de como executar, informações gerais
├── Makefile              
├── .gitignore            
├── src/                  ← Pasta onde ficam os códigos do projeto
│   ├── main_no.cpp       ← stub
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

