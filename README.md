# Bomberman em C++

Este projeto e uma versao em console do jogo Bomberman, desenvolvida em C++ para Windows. O jogador deve explorar mapas em formato de matriz, destruir caixas, coletar power-ups, eliminar inimigos e encontrar o portal para avancar de fase. O jogo possui campanha com varias fases, sistema de pontuacao, ranking, modo para dois jogadores e tambem um modo em que um robo joga automaticamente.

## Objetivo do jogo

O objetivo principal e sobreviver, derrotar todos os inimigos da fase e entrar no portal que aparece no mapa. O jogador usa bombas para destruir caixas quebraveis e atingir inimigos. Depois que todos os inimigos sao eliminados, o portal aparece e permite seguir para a proxima fase.

Na campanha, o jogador passa por quatro fases, incluindo uma fase final com boss. Ao final da campanha ou em caso de derrota, o jogo calcula a pontuacao e salva o resultado no ranking.

## Linguagem e tecnologias

- Linguagem principal: C++
- Padrao configurado no projeto: C++20
- Plataforma: Windows
- Build system: CMake
- Bibliotecas/headers usados:
  - `windows.h` para recursos do console e funcoes do Windows
  - `conio.h` para leitura de teclas com `getch()` e `_kbhit()`
  - `mmsystem.h` e `winmm` para reproducao de sons
  - STL do C++: `vector`, `queue`, `string`, `fstream`, `algorithm`, entre outros

## Principais funcionalidades

- Menu principal com opcoes de jogo, instrucoes, dificuldades, sistema de pontuacao e ranking.
- Campanha com quatro fases.
- Tres dificuldades:
  - Facil: inimigos mais lentos e aleatorios.
  - Medio: inimigos com chance de perseguir o jogador.
  - Dificil: inimigos perseguem com mais frequencia.
- Modo de um jogador.
- Modo de dois jogadores no mesmo mapa.
- Modo robo, em que a IA joga sozinha.
- Sistema de bombas com raio de explosao.
- Caixas quebraveis que podem liberar power-ups.
- Portal para avancar de fase.
- Boss na fase final.
- Sistema de vidas.
- Sistema de pontuacao individual.
- Ranking salvo em arquivo `ranking.txt`.
- Sons de intro, movimento, inimigos, itens, explosao, eliminacao e trilhas de fase.

## Modo robo

O modo robo foi implementado para permitir que o jogo seja assistido automaticamente. O robo analisa o mapa, procura alvos e toma decisoes sozinho.

Ele consegue:

- Procurar inimigos.
- Procurar caixas quebraveis.
- Escolher posicoes de ataque onde a bomba pode acertar inimigos ou caixas.
- Plantar bombas quando existe rota de fuga.
- Fugir da area de explosao depois de plantar bomba.
- Evitar posicoes perigosas.
- Buscar power-ups visiveis quando o caminho esta seguro.
- Ir ate o portal depois que todos os inimigos forem derrotados.

## Inimigos

Os inimigos se movimentam pelo mapa e podem perseguir o jogador dependendo da dificuldade. Tambem foi implementado controle para impedir que dois inimigos ocupem a mesma casa.

Essa verificacao acontece:

- Ao criar os inimigos no inicio da fase.
- Durante o movimento dos inimigos.
- No teleporte do boss.

## Power-ups

Os power-ups aparecem ao destruir caixas quebraveis. Cada item melhora alguma caracteristica do jogador:

- Fogo: aumenta o raio da explosao da bomba.
- Bomba+: permite deixar mais bombas ativas ao mesmo tempo.
- Patins: aumenta a velocidade de movimento.
- Vida: adiciona uma vida extra.
- Relogio: faz as bombas explodirem mais rapido.
- Escudo: bloqueia um dano recebido e depois desaparece.
- Skate: permite atravessar caixas quebraveis.

## Controles

### Jogador 1

- `W`: mover para cima
- `S`: mover para baixo
- `A`: mover para esquerda
- `D`: mover para direita
- `Espaco`: plantar bomba

### Jogador 2

- `Seta para cima`: mover para cima
- `Seta para baixo`: mover para baixo
- `Seta para esquerda`: mover para esquerda
- `Seta para direita`: mover para direita
- `Enter`: plantar bomba

### Pause

- `P` ou `ESC`: pausar o jogo

## Pontuacao

A pontuacao e calculada de forma dinamica:

- +100 pontos por inimigo abatido
- +20 pontos por caixa destruida
- -1 ponto por movimento
- -2 pontos por bomba utilizada

Durante a partida, o HUD mostra a pontuacao atual, movimentos feitos, vidas, quantidade de bombas e raio de fogo. Ao final da campanha, o jogo mostra o resumo com pontuacao final, movimentos, bombas usadas, inimigos abatidos e caixas destruidas.

## Estrutura do projeto

```text
bomberman/
├── CMakeLists.txt
├── main.cpp
├── README.md
└── resources/
    ├── mapas.cpp
    ├── mapas.h
    └── sounds/
```

## Como compilar

O projeto usa CMake. Em um terminal dentro da pasta do projeto, execute:

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
```

Tambem e possivel compilar manualmente com `g++`, linkando a biblioteca `winmm`:

```bash
g++ -std=c++20 main.cpp resources/mapas.cpp -lwinmm -o bomberman.exe
```

## Observacoes

Este projeto foi feito como um jogo de console, usando matriz para representar o mapa. Mesmo sem interface grafica tradicional, ele possui varias mecanicas de jogo, como IA, ranking, audio, power-ups, fases, boss e diferentes modos de jogo.

O codigo principal esta em `main.cpp`, enquanto os mapas ficam separados em `resources/mapas.cpp` e `resources/mapas.h`.
