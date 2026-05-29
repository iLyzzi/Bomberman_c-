#include <iostream>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <queue>
#include <mmsystem.h>

#include "resources/mapas.h"
#include <cstdint>

using namespace std;

// --- VARIÁVEIS GLOBAIS DE ÁUDIO ---
string pathIntro = "";
string pathMove = "";
string pathEnemy = "";
string pathItem = "";
string pathExplode = "";
string pathKill = "";
vector<string> pathBGMs;

// --- FUNÇÕES DE ÁUDIO ---
string encontrarSom(string caminhoRelativo) {
    for (size_t i = 0; i < caminhoRelativo.length(); i++) {
        if (caminhoRelativo[i] == '/') caminhoRelativo[i] = '\\';
    }
    ifstream f(caminhoRelativo);
    if (f.good()) return caminhoRelativo;
    
    string path2 = "..\\" + caminhoRelativo;
    ifstream f2(path2);
    if (f2.good()) return path2;

    string path3 = "..\\..\\" + caminhoRelativo;
    ifstream f3(path3);
    if (f3.good()) return path3;

    return caminhoRelativo;
}

string obterCaminhoMCI(const string& caminhoRelativo) {
    char buffer[MAX_PATH];
    DWORD retval = GetFullPathNameA(caminhoRelativo.c_str(), MAX_PATH, buffer, NULL);
    string absPath = (retval == 0) ? caminhoRelativo : string(buffer);
    
    char shortBuffer[MAX_PATH];
    DWORD shortRetval = GetShortPathNameA(absPath.c_str(), shortBuffer, MAX_PATH);
    if (shortRetval == 0) return absPath;
    return string(shortBuffer);
}

void pararTudoMCI() {
    mciSendStringA("stop intro", NULL, 0, NULL);
    mciSendStringA("close intro", NULL, 0, NULL);
    mciSendStringA("stop bgm", NULL, 0, NULL);
    mciSendStringA("close bgm", NULL, 0, NULL);
}

void inicializarAudio() {
    pathIntro = encontrarSom("resources\\sounds\\intro.wav");
    pathMove = encontrarSom("resources\\sounds\\move.wav");
    pathEnemy = encontrarSom("resources\\sounds\\enemy.wav");
    pathItem = encontrarSom("resources\\sounds\\item.wav");
    pathExplode = encontrarSom("resources\\sounds\\explodir.wav");
    pathKill = encontrarSom("resources\\sounds\\kill.wav");

    // Fases do jogo
    pathBGMs.clear();
    pathBGMs.push_back(encontrarSom("resources\\sounds\\bgm1.wav")); // Fase 1
    pathBGMs.push_back(encontrarSom("resources\\sounds\\bgm2.wav")); // Fase 2
    pathBGMs.push_back(encontrarSom("resources\\sounds\\bgm3.wav")); // Fase 3
    pathBGMs.push_back(encontrarSom("resources\\sounds\\bgm-boss.wav")); // Boss
}

void fecharAudio() {
    pararTudoMCI();
    PlaySoundA(NULL, NULL, 0);
}

void tocarIntro() {
    pararTudoMCI();
    string mciPath = obterCaminhoMCI(pathIntro);
    string cmdOpen = "open " + mciPath + " type mpegvideo alias intro";
    mciSendStringA(cmdOpen.c_str(), NULL, 0, NULL);
    mciSendStringA("play intro repeat", NULL, 0, NULL);
}

void pararIntro() {
    mciSendStringA("stop intro", NULL, 0, NULL);
    mciSendStringA("close intro", NULL, 0, NULL);
}

void tocarBGM(int fase) {
    pararTudoMCI();
    if (fase < 0 || fase >= (int)pathBGMs.size()) return;
    
    string mciPath = obterCaminhoMCI(pathBGMs[fase]);
    string cmdOpen = "open " + mciPath + " type mpegvideo alias bgm";
    mciSendStringA(cmdOpen.c_str(), NULL, 0, NULL);
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
}

void pararBGM() {
    mciSendStringA("stop bgm", NULL, 0, NULL);
    mciSendStringA("close bgm", NULL, 0, NULL);
}

void tocarMovimento() {
    PlaySoundA(pathMove.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

void tocarInimigo() {
    PlaySoundA(pathEnemy.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

void tocarItem() {
    PlaySoundA(pathItem.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

void tocarExplosao() {
    PlaySoundA(pathExplode.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

void tocarKill() {
    PlaySoundA(pathKill.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}



#define TAM 15
#define quantidadeFases 4
#define quantidadeDificuldades 3


// --- VARIÁVEIS GLOBAIS DE PONTUAÇÃO ---
int movimentos = 0;
int bombasUsadas = 0;
int inimigosAbatidos = 0;
int caixasDestruidas = 0;
int tempoCampanhaGasto = 0;

// Controle da campanha e ranking individual
int quantidadeJogadoresCampanha = 1;
string playerNames[2] = {"", ""};
int movimentosJogador[2] = {0, 0};
int bombasUsadasJogador[2] = {0, 0};
int inimigosAbatidosJogador[2] = {0, 0};
int caixasDestruidasJogador[2] = {0, 0};

// Modo de jogo: 1 = humano sozinho, 2 = dois humanos, 3 = robo sozinho
int modoJogoCampanha = 1;
bool roboAtivo = false;
string acaoRoboAtual = "Aguardando inicio";
DWORD ultimoMovimentoRobo = 0;
DWORD ultimoPlantioRobo = 0;

// --- ESTRUTURAS ---
struct Ranking {
    string playerName;
    int pontuacao;
    int bombas;
    int movimentos;
    int caixas;
    int tempo;
    string data;
};

struct Coordenadas {
    int X, Y;
};

struct Explosao {
    Coordenadas posicao;
    bool explodindo = false;
    DWORD tempoExplodindo = 0;
};

struct Bomba {
    Coordenadas posicao;
    bool bombaAtiva = false;
    DWORD tempobomba = 0;
    Explosao explosao;
};

struct Inimigo {
    Coordenadas posicao;
    int direcao = 0;
    DWORD tempoMover = 0;
    bool isBoss = false;
    DWORD tempoTeleporte = 0;
    DWORD tempoSpawnBomba = 0;
    int vidas = 1;
};

struct Personagem {
    Coordenadas posicao;
    bool hp = true;
    int vidas = 1;
    int raioBomba = 1;
    int maxBombas = 1;
    int velocidade = 1;
    bool bombaRelogio = false;
    bool invulneravel = false;
    bool fantasma = false;
    DWORD tempoMover = 0;
};

// --- MAPAS DO JOGO ---
int mapasDificuldade[quantidadeDificuldades][quantidadeFases][TAM][TAM];

// --- PROTÓTIPOS DAS FUNÇÕES ---
void posicionarPortal(int mapaAtual[TAM][TAM]);
bool iniciarJogo(int dificuldade, int fase);
int escolherQuantidadeJogadores();
void exibirRanking();
void exibirSistemaPontuacao();
void exibirDificuldadesEItens();
void exibirComoFunciona();
void pausarParaVoltar();
void exibirMenuPrincipal();
bool compararPontuacao(const Ranking &a, const Ranking &b);
void moverCursorParaInicio();
void limparTela();
void exibirTelaVitoria();
void exibirTelaDerrota();
bool verificarVitoria(Inimigo inimigos[], int qtd);
void limparExplosao(int mapaAtual[TAM][TAM]);
void desenharExplosao(int mapaAtual[TAM][TAM], int fogoX, int fogoY, Personagem jogadores[], int qtdJogadores, int dono, Inimigo inimigos[], int qtd, int raio);
void gerenciarBombasJogador(int mapaAtual[TAM][TAM], vector<Bomba> &bombasJogador, Personagem jogadores[], int qtdJogadores, int dono, Inimigo inimigos[], int quantidadeInimigos);
void moverInimigo(int mapaAtual[TAM][TAM], Inimigo inimigos[], int indice, int quantidadeInimigos, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, Personagem jogadores[], int qtdJogadores, int dificuldade);
bool existeBombaNaPosicao(const vector<Bomba> &bombas, int x, int y);
bool existeBombaNaPosicaoTodas(const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, int x, int y);
bool verificaCaminho(int mapaAtual[TAM][TAM], int x, int y, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, bool fantasma = false);
int calcularPontosJogador(int indice);
bool existeJogadorVivo(Personagem jogadores[], int qtdJogadores);
void renderizarHUD(Personagem jogadores[], int qtdJogadores, int tempoDecorrido, Inimigo inimigos[], int qtd);
void renderizarJogo(int mapaAtual[TAM][TAM], Personagem jogadores[], int qtdJogadores, Inimigo inimigos[], int qtd, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, vector<Bomba> &bombasBoss);
void renderizarLinhasRanking();
void atualizarRanking(const string &playerName, int totalPontos, int tempoGasto, int jogador);
void gerarCaixasAleatorias(int mapaAtual[TAM][TAM], int chanceCaixa);
bool posicaoProtegida(int linha, int coluna);
void limparAreasProtegidasJogadores(int mapaAtual[TAM][TAM]);
bool todosJogadoresPassaramPortal(bool jogadoresNoPortal[], Personagem jogadores[], int qtdJogadores);
void exibirTelaFinal(int tempoGasto, bool venceuCampanha);
int escolherDificuldade();
void acoesBoss(int mapaAtual[TAM][TAM], Inimigo inimigos[], int indiceBoss, int quantidadeInimigos, Personagem jogadores[], int qtdJogadores, vector<Bomba> &bombasBoss);
void gerenciarBombasBoss(int mapaAtual[TAM][TAM], vector<Bomba> &bombasBoss, Personagem jogadores[], int qtdJogadores, Inimigo inimigos[], int qtd);
void ajustarRelogiosDepoisPause(DWORD pausaDuracao, DWORD &tempoInicio, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, vector<Bomba> &bombasBoss, Inimigo inimigos[], int quantidadeInimigos);
void pausarJogo(DWORD &tempoInicio, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, vector<Bomba> &bombasBoss, Inimigo inimigos[], int quantidadeInimigos);
void propagarFogoRecursivo(int mapaAtual[TAM][TAM], int x, int y, int raio, int dx, int dy, int dono);
bool posicaoPerigosaParaRobo(int mapaAtual[TAM][TAM], int x, int y, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, int raio, int distanciaMinimaInimigo = 1);
bool existeRotaDeFugaParaRobo(int mapaAtual[TAM][TAM], Personagem &robo, vector<Bomba> bombasP1, vector<Bomba> bombasP2, Inimigo inimigos[], int quantidadeInimigos);
int escolherMelhorDirecaoRobo(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, Coordenadas alvo, bool fugindo);
int escolherDirecaoFugaRoboBFS(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos);
int escolherDirecaoAlvoRoboBFS(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, Coordenadas alvo);
bool bombaDoRoboAcertaInimigo(int mapaAtual[TAM][TAM], Coordenadas posicaoBomba, Inimigo inimigos[], int quantidadeInimigos, int raio);
bool bombaDoRoboAcertaCaixa(int mapaAtual[TAM][TAM], Coordenadas posicaoBomba, int raio);
bool existeRotaDeFugaParaRoboNaPosicao(int mapaAtual[TAM][TAM], Personagem robo, Coordenadas posicao, vector<Bomba> bombasP1, vector<Bomba> bombasP2, Inimigo inimigos[], int quantidadeInimigos);
Coordenadas escolherPosicaoAtaqueRobo(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos);
bool existeInimigoNaPosicao(Inimigo inimigos[], int quantidadeInimigos, int x, int y, int ignorarIndice = -1);
bool ehPowerUp(int valorMapa);
Coordenadas escolherPowerUpSeguroRobo(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos);
bool tentarMoverJogador(int jogador, int dx, int dy, Personagem jogadores[], int qtdJogadores, int mapaAtual[TAM][TAM], vector<Bomba> &bombasP1, vector<Bomba> &bombasP2);
bool plantarBombaJogador(int jogador, Personagem jogadores[], vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, int mapaAtual[TAM][TAM]);
void executarRobo(int mapaAtual[TAM][TAM], Personagem jogadores[], int qtdJogadores, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, bool portalAtivo, Coordenadas portalPos);


template <typename T>
T calcularPontuacaoDinamica(T abatidos, T caixas, T movs, T bombas) {
    T pontos = (abatidos * 100) + (caixas * 20) - movs - (bombas * 2);
    return pontos < 0 ? 0 : pontos;
}

void exibirMensagemStatus(string msg) {
    cout << "\n> " << msg << " <\n";
}
void exibirMensagemStatus(string msg, int delayMs) {
    (void)delayMs;
    cout << "\n> " << msg << " <\n";
}

bool iniciarJogo(int dificuldade, int fase) {
    bool novaCampanha = (fase == 0);
    if (playerNames[0] == "") {
        limparTela();
        if (roboAtivo) {
            cout << "Digite o nickname do Robo para o ranking: ";
            cin >> playerNames[0];
            acaoRoboAtual = "Analisando o mapa";
        } else {
            cout << "Digite o nickname do Jogador 1: ";
            cin >> playerNames[0];
            if (quantidadeJogadoresCampanha == 2) {
                cout << "Digite o nickname do Jogador 2: ";
                cin >> playerNames[1];
            }
        }
    }

    // Cheat code: se o nickname for "boss", pula direto para a fase final do Boss (fase 3)
    if (playerNames[0] == "boss" || playerNames[0] == "BOSS" || playerNames[0] == "Boss") {
        fase = 3;
    } else if (playerNames[0] == "phase2" || playerNames[0] == "PHASE2" || playerNames[0] == "Phase2") {
        fase = 2;
    }

    bool venceuFase = false;
    int mapaAtual[TAM][TAM];
    int quantidadeInimigos;
    bool portalAtivo = false;
    Coordenadas portalPos = {-1, -1};

    for(int i = 0; i < TAM; i++) {
        for(int j = 0; j < TAM; j++) {
            if (dificuldade == 0) {
                switch(fase) {
                    case 0: mapaAtual[i][j] = easyFase1[i][j]; break;
                    case 1: mapaAtual[i][j] = easyFase2[i][j]; break;
                    case 2: mapaAtual[i][j] = easyFase3[i][j]; break;
                    case 3: mapaAtual[i][j] = easyFase4[i][j]; break;
                }
            }
            else if (dificuldade == 1) {
                switch(fase) {
                    case 0: mapaAtual[i][j] = mediumFase1[i][j]; break;
                    case 1: mapaAtual[i][j] = mediumFase2[i][j]; break;
                    case 2: mapaAtual[i][j] = mediumFase3[i][j]; break;
                    case 3: mapaAtual[i][j] = mediumFase4[i][j]; break;
                }
            }
            else if (dificuldade == 2) {
                switch(fase) {
                    case 0: mapaAtual[i][j] = hardFase1[i][j]; break;
                    case 1: mapaAtual[i][j] = hardFase2[i][j]; break;
                    case 2: mapaAtual[i][j] = hardFase3[i][j]; break;
                    case 3: mapaAtual[i][j] = hardFase4[i][j]; break;
                }
            }
        }
    }

    if (dificuldade == 0) { gerarCaixasAleatorias(mapaAtual, 25); quantidadeInimigos = 3; }
    else if (dificuldade == 1) { gerarCaixasAleatorias(mapaAtual, 35); quantidadeInimigos = 5; }
    else { gerarCaixasAleatorias(mapaAtual, 45); quantidadeInimigos = 7; }

    // Garante que as areas iniciais dos jogadores fiquem livres mesmo se a matriz da fase
    // ou a geracao aleatoria tiver colocado blocos quebraveis perto do nascimento.
    limparAreasProtegidasJogadores(mapaAtual);

    portalAtivo = false;
    portalPos = {-1, -1};

    static Personagem jogadores[2];
    for (int i = 0; i < 2; i++) {
        jogadores[i].posicao = (i == 0 ? Coordenadas{1, 1} : Coordenadas{1, 13});
        if (novaCampanha) {
            jogadores[i].hp = true;
            jogadores[i].vidas = 1;
        }
        jogadores[i].raioBomba = 1;
        jogadores[i].maxBombas = 1;
        jogadores[i].velocidade = 1;
        jogadores[i].bombaRelogio = false;
        jogadores[i].invulneravel = false;
        jogadores[i].fantasma = false;
        jogadores[i].tempoMover = 0;
    }

    Inimigo* inimigos = new Inimigo[quantidadeInimigos];
    for (int i = 0; i < quantidadeInimigos; i++) {
        int x, y;
        do {
            x = rand() % TAM;
            y = rand() % TAM;
        } while (mapaAtual[x][y] != 0 || posicaoProtegida(x, y) || existeInimigoNaPosicao(inimigos, i, x, y));

        inimigos[i].posicao.X = x;
        inimigos[i].posicao.Y = y;
        inimigos[i].tempoMover = 0;
        inimigos[i].tempoTeleporte = 0;
        inimigos[i].tempoSpawnBomba = 0;
        inimigos[i].direcao = rand() % 4;

        if (fase == 3 && i == 0) {
            inimigos[i].isBoss = true;
            inimigos[i].vidas = 3;
        } else {
            inimigos[i].isBoss = false;
            inimigos[i].vidas = 1;
        }
    }

    vector<Bomba> bombasP1;
    vector<Bomba> bombasP2;
    vector<Bomba> bombasBoss;
    bool jogoAtivo = true;
    bool jogadoresNoPortal[2] = {false, false};

    DWORD tempoInicio = GetTickCount();

    limparTela();
    cout << "Fase " << fase + 1 << "\n";
    if (fase == 3) cout << "CUIDADO! O Boss esta a solta!\n";
    cout << "Iniciando o jogo... Prepare-se!\n";
    if (quantidadeJogadoresCampanha == 2) {
        cout << "J1: WASD + ESPACO | J2: SETAS + ENTER\n";
    } else {
        cout << "J1: WASD + ESPACO\n";
    }
    exibirMensagemStatus("Pressione qualquer tecla para começar...", 0);
    getch();
    limparTela();

    tocarBGM(fase);

    while(jogoAtivo) {
        if (!portalAtivo && verificarVitoria(inimigos, quantidadeInimigos)) {
            posicionarPortal(mapaAtual);
            for (int i = 0; i < TAM; i++) {
                for (int j = 0; j < TAM; j++) if (mapaAtual[i][j] == 5) portalPos = {i, j};
            }
            portalAtivo = true;
        }

        int tempoDecorrido = tempoCampanhaGasto + ((GetTickCount() - tempoInicio) / 1000);
        moverCursorParaInicio();
        renderizarHUD(jogadores, quantidadeJogadoresCampanha, tempoDecorrido, inimigos, quantidadeInimigos);
        renderizarJogo(mapaAtual, jogadores, quantidadeJogadoresCampanha, inimigos, quantidadeInimigos, bombasP1, bombasP2, bombasBoss);

        if (_kbhit()) {
            char tecla = getch();
            int jogador = -1;
            int dx = 0, dy = 0;
            bool moveu = false;
            bool plantar = false;

            if(tecla == 'p' || tecla == 'P' || tecla == 27) {
                pausarJogo(tempoInicio, bombasP1, bombasP2, bombasBoss, inimigos, quantidadeInimigos);
                continue;
            }

            if(tecla == 'w' || tecla == 'W') { jogador = 0; dx = -1; moveu = true; }
            else if(tecla == 's' || tecla == 'S') { jogador = 0; dx = 1; moveu = true; }
            else if(tecla == 'a' || tecla == 'A') { jogador = 0; dy = -1; moveu = true; }
            else if(tecla == 'd' || tecla == 'D') { jogador = 0; dy = 1; moveu = true; }
            else if(tecla == ' ') { jogador = 0; plantar = true; }
            else if(tecla == 13 && quantidadeJogadoresCampanha == 2) { jogador = 1; plantar = true; } // ENTER

            //jogador 2
            else if ((tecla == 0 || tecla == -32 || tecla == 224) && quantidadeJogadoresCampanha == 2) {
                char especial = getch();
                jogador = 1;
                if (especial == 72) { dx = -1; moveu = true; } // UP
                else if (especial == 80) { dx = 1; moveu = true; } // DOWN
                else if (especial == 75) { dy = -1; moveu = true; } // LEFT
                else if (especial == 77) { dy = 1; moveu = true; } // RIGHT
                else if (especial == 13) { plantar = true; } // ENTER numerico
                else jogador = -1;
            }

            if (jogador >= 0 && jogador < quantidadeJogadoresCampanha && jogadores[jogador].hp && !jogadoresNoPortal[jogador]) {
                vector<Bomba> &bombasDoJogador = (jogador == 0 ? bombasP1 : bombasP2);

                if (plantar) {
                    plantarBombaJogador(jogador, jogadores, bombasP1, bombasP2, mapaAtual);
                }

                if(moveu) {
                    DWORD agora = GetTickCount();
                    int delayMovimento = 250 - (jogadores[jogador].velocidade * 40);
                    if (delayMovimento < 50) delayMovimento = 50;

                    if (agora - jogadores[jogador].tempoMover >= (DWORD)delayMovimento) {
                        if (tentarMoverJogador(jogador, dx, dy, jogadores, quantidadeJogadoresCampanha, mapaAtual, bombasP1, bombasP2)) {
                            jogadores[jogador].tempoMover = agora;
                        }
                    }
                }
            }
        }

        if (roboAtivo) {
            executarRobo(mapaAtual, jogadores, quantidadeJogadoresCampanha, bombasP1, bombasP2, inimigos, quantidadeInimigos, portalAtivo, portalPos);
        }

        gerenciarBombasJogador(mapaAtual, bombasP1, jogadores, quantidadeJogadoresCampanha, 0, inimigos, quantidadeInimigos);
        if (quantidadeJogadoresCampanha == 2) {
            gerenciarBombasJogador(mapaAtual, bombasP2, jogadores, quantidadeJogadoresCampanha, 1, inimigos, quantidadeInimigos);
        }
        gerenciarBombasBoss(mapaAtual, bombasBoss, jogadores, quantidadeJogadoresCampanha, inimigos, quantidadeInimigos);

        for(int i = 0; i < quantidadeInimigos; i++) {
            moverInimigo(mapaAtual, inimigos, i, quantidadeInimigos, bombasP1, bombasP2, jogadores, quantidadeJogadoresCampanha, dificuldade);

            if (inimigos[i].isBoss && inimigos[i].posicao.X != -1) {
                acoesBoss(mapaAtual, inimigos, i, quantidadeInimigos, jogadores, quantidadeJogadoresCampanha, bombasBoss);
            }

            for (int j = 0; j < quantidadeJogadoresCampanha; j++) {
                if (jogadores[j].hp && !jogadoresNoPortal[j] && jogadores[j].posicao.X == inimigos[i].posicao.X && jogadores[j].posicao.Y == inimigos[i].posicao.Y && inimigos[i].posicao.X != -1) {
                    if(jogadores[j].invulneravel) {
                        jogadores[j].invulneravel = false;
                        inimigos[i].posicao.X = -1;
                    } else {
                        jogadores[j].vidas--;
                        if(jogadores[j].vidas <= 0) jogadores[j].hp = false;
                        else {
                            jogadores[j].posicao = (j == 0 ? Coordenadas{1,1} : Coordenadas{1,13});
                            Beep(300, 200);
                            Beep(200, 300);
                        }
                    }
                }
            }
        }

        if (portalAtivo) {
            for (int j = 0; j < quantidadeJogadoresCampanha; j++) {
                if (!jogadoresNoPortal[j] && jogadores[j].hp && jogadores[j].posicao.X == portalPos.X && jogadores[j].posicao.Y == portalPos.Y) {
                    jogadoresNoPortal[j] = true;

                    if (quantidadeJogadoresCampanha == 2) {
                        jogadores[j].posicao = {-1, -1}; // tira do mapa para nao bloquear a porta do outro jogador
                        exibirMensagemStatus("Jogador " + to_string(j + 1) + " entrou no portal. Aguardando o outro jogador...", 700);
                        cout << "Pressione qualquer tecla para continuar.";
                        getch();
                    }
                }
            }

            if (todosJogadoresPassaramPortal(jogadoresNoPortal, jogadores, quantidadeJogadoresCampanha)) {
                venceuFase = true;
                exibirTelaVitoria();
                jogoAtivo = false;
            }
        }

        // Fim de jogo por derrota: nenhum jogador vivo (ou o robô faleceu no modo de simulação autônoma)
        if (!existeJogadorVivo(jogadores, quantidadeJogadoresCampanha) || (roboAtivo && !jogadores[0].hp)) {
            jogoAtivo = false;
        }

        if (!jogoAtivo && !venceuFase) {
            exibirTelaDerrota();
            cout << "\nPressione qualquer tecla para continuar.";
            getch();
        }
    }

    limparTela();
    pararBGM();

    int tempoGasto = (GetTickCount() - tempoInicio) / 1000;
    tempoCampanhaGasto += tempoGasto;

    if (!venceuFase || fase == quantidadeFases - 1) {
        for (int i = 0; i < quantidadeJogadoresCampanha; i++) {
            atualizarRanking(playerNames[i], calcularPontosJogador(i), tempoCampanhaGasto, i);
        }
        bool venceuCampanha = (venceuFase && (fase == quantidadeFases - 1));
        exibirTelaFinal(tempoCampanhaGasto, venceuCampanha);
    }

    delete[] inimigos;
    return venceuFase;
}



void ajustarRelogiosDepoisPause(DWORD pausaDuracao, DWORD &tempoInicio, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, vector<Bomba> &bombasBoss, Inimigo inimigos[], int quantidadeInimigos) {
    tempoInicio += pausaDuracao;
    for (auto &b : bombasP1) { b.tempobomba += pausaDuracao; if (b.explosao.explodindo) b.explosao.tempoExplodindo += pausaDuracao; }
    for (auto &b : bombasP2) { b.tempobomba += pausaDuracao; if (b.explosao.explodindo) b.explosao.tempoExplodindo += pausaDuracao; }
    for (auto &b : bombasBoss) { b.tempobomba += pausaDuracao; if (b.explosao.explodindo) b.explosao.tempoExplodindo += pausaDuracao; }
    for (int i = 0; i < quantidadeInimigos; i++) {
        inimigos[i].tempoMover += pausaDuracao;
        inimigos[i].tempoTeleporte += pausaDuracao;
        inimigos[i].tempoSpawnBomba += pausaDuracao;
    }
    ultimoMovimentoRobo += pausaDuracao;
    ultimoPlantioRobo += pausaDuracao;
}

void pausarJogo(DWORD &tempoInicio, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, vector<Bomba> &bombasBoss, Inimigo inimigos[], int quantidadeInimigos) {
    DWORD inicioPause = GetTickCount();
    limparTela();
    cout << "=========================================" << endl;
    cout << "              JOGO PAUSADO              " << endl;
    cout << "=========================================" << endl;
    cout << " O tempo, os inimigos, as bombas e o robo" << endl;
    cout << " ficam parados enquanto o jogo esta pausado." << endl;
    cout << "\n Pressione P ou ESC para voltar ao jogo." << endl;

    while (true) {
        char tecla = getch();
        if (tecla == 'p' || tecla == 'P' || tecla == 27) break;
    }
    DWORD duracaoPause = GetTickCount() - inicioPause;
    ajustarRelogiosDepoisPause(duracaoPause, tempoInicio, bombasP1, bombasP2, bombasBoss, inimigos, quantidadeInimigos);
    limparTela();
}

bool existeBombaNaPosicao(const vector<Bomba> &bombas, int x, int y) {
    for (const auto &b : bombas) {
        if (b.bombaAtiva && b.posicao.X == x && b.posicao.Y == y) return true;
    }
    return false;
}

bool existeBombaNaPosicaoTodas(const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, int x, int y) {
    return existeBombaNaPosicao(bombasP1, x, y) || existeBombaNaPosicao(bombasP2, x, y);
}

bool verificaCaminho(int mapaAtual[TAM][TAM], int x, int y, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, bool fantasma) {
    if (x < 0 || x >= TAM || y < 0 || y >= TAM) return false;
    if (mapaAtual[x][y] == 1) return false;
    if (!fantasma && mapaAtual[x][y] == 2) return false;
    if (mapaAtual[x][y] == 4) return false;
    if (existeBombaNaPosicaoTodas(bombasP1, bombasP2, x, y)) return false;
    return true;
}

void propagarFogoRecursivo(int mapaAtual[TAM][TAM], int x, int y, int raio, int dx, int dy, int dono) {
    if (raio <= 0) return;
    int nx = x + dx;
    int ny = y + dy;
    if (nx < 0 || nx >= TAM || ny < 0 || ny >= TAM) return;
    if (mapaAtual[nx][ny] == 1) return;

    if (mapaAtual[nx][ny] == 2) {
        caixasDestruidas++;
        if (dono >= 0 && dono < 2) caixasDestruidasJogador[dono]++;
        if (rand() % 100 < 40) mapaAtual[nx][ny] = (rand() % 7) + 6;
        else mapaAtual[nx][ny] = 4;
        
        // A explosão da bomba passa pelo bloco quebrável e continua propagando de acordo com o raio restante
        propagarFogoRecursivo(mapaAtual, nx, ny, raio - 1, dx, dy, dono);
        return;
    }

    if (mapaAtual[nx][ny] != 5) mapaAtual[nx][ny] = 4;
    propagarFogoRecursivo(mapaAtual, nx, ny, raio - 1, dx, dy, dono);
}

void limparExplosao(int mapaAtual[TAM][TAM]) {
    for (int i = 0; i < TAM; i++) for (int j = 0; j < TAM; j++) if (mapaAtual[i][j] == 4) mapaAtual[i][j] = 0;
}

void desenharExplosao(int mapaAtual[TAM][TAM], int fogoX, int fogoY, Personagem jogadores[], int qtdJogadores, int dono, Inimigo inimigos[], int qtd, int raio) {
    tocarExplosao();
    mapaAtual[fogoX][fogoY] = 4;
    propagarFogoRecursivo(mapaAtual, fogoX, fogoY, raio, 1, 0, dono);
    propagarFogoRecursivo(mapaAtual, fogoX, fogoY, raio, -1, 0, dono);
    propagarFogoRecursivo(mapaAtual, fogoX, fogoY, raio, 0, 1, dono);
    propagarFogoRecursivo(mapaAtual, fogoX, fogoY, raio, 0, -1, dono);

    for (int k = 0; k < qtd; k++) {
        if (inimigos[k].posicao.X != -1 && mapaAtual[inimigos[k].posicao.X][inimigos[k].posicao.Y] == 4) {
            inimigos[k].vidas--;
            if (inimigos[k].vidas <= 0) {
                inimigos[k].posicao.X = -1;
                inimigosAbatidos++;
                if (dono >= 0 && dono < 2) inimigosAbatidosJogador[dono]++;
                tocarKill();
            } else if (inimigos[k].isBoss) {
                inimigos[k].tempoTeleporte = 0;
            }
        }
    }

    for (int j = 0; j < qtdJogadores; j++) {
        if (jogadores[j].hp && mapaAtual[jogadores[j].posicao.X][jogadores[j].posicao.Y] == 4) {
            if (jogadores[j].invulneravel) jogadores[j].invulneravel = false;
            else {
                jogadores[j].vidas--;
                if (jogadores[j].vidas <= 0) jogadores[j].hp = false;
            }
        }
    }
}

void acoesBoss(int mapaAtual[TAM][TAM], Inimigo inimigos[], int indiceBoss, int quantidadeInimigos, Personagem jogadores[], int qtdJogadores, vector<Bomba> &bombasBoss) {
    Inimigo &boss = inimigos[indiceBoss];
    DWORD agora = GetTickCount();
    if (agora - boss.tempoTeleporte > 5000) {
        vector<Coordenadas> livres;
        for (int i = 1; i < TAM - 1; i++) for (int j = 1; j < TAM - 1; j++) {
            if (mapaAtual[i][j] == 0 && !existeInimigoNaPosicao(inimigos, quantidadeInimigos, i, j, indiceBoss)) {
                bool longe = true;
                for (int p = 0; p < qtdJogadores; p++) if (jogadores[p].hp && abs(i - jogadores[p].posicao.X) <= 2 && abs(j - jogadores[p].posicao.Y) <= 2) longe = false;
                if (longe) livres.push_back({i, j});
            }
        }
        if (!livres.empty()) {
            Coordenadas novaPos = livres[rand() % livres.size()];
            if (novaPos.X > 0 && novaPos.X < TAM - 1 && novaPos.Y > 0 && novaPos.Y < TAM - 1) {
                boss.posicao = novaPos;
            }
        }
        boss.tempoTeleporte = agora;
    }
}

void gerenciarBombasBoss(int mapaAtual[TAM][TAM], vector<Bomba> &bombasBoss, Personagem jogadores[], int qtdJogadores, Inimigo inimigos[], int qtd) {
    DWORD agora = GetTickCount();
    for (auto it = bombasBoss.begin(); it != bombasBoss.end(); ) {
        if (it->bombaAtiva && agora - it->tempobomba > 2000) {
            it->bombaAtiva = false;
            it->explosao.explodindo = true;
            it->explosao.posicao = it->posicao;
            it->explosao.tempoExplodindo = agora;
            desenharExplosao(mapaAtual, it->posicao.X, it->posicao.Y, jogadores, qtdJogadores, -1, inimigos, qtd, 3);
        }
        if (it->explosao.explodindo && agora - it->explosao.tempoExplodindo > 500) {
            limparExplosao(mapaAtual);
            it = bombasBoss.erase(it);
            continue;
        }
        ++it;
    }
}

void gerenciarBombasJogador(int mapaAtual[TAM][TAM], vector<Bomba> &bombasJogador, Personagem jogadores[], int qtdJogadores, int dono, Inimigo inimigos[], int quantidadeInimigos) {
    DWORD agora = GetTickCount();
    for (auto it = bombasJogador.begin(); it != bombasJogador.end(); ) {
        if (it->bombaAtiva) {
            DWORD tempoDetonacao = jogadores[dono].bombaRelogio ? 1000 : 2500;
            if (agora - it->tempobomba > tempoDetonacao) {
                it->bombaAtiva = false;
                it->explosao.explodindo = true;
                it->explosao.posicao = it->posicao;
                it->explosao.tempoExplodindo = agora;
                desenharExplosao(mapaAtual, it->posicao.X, it->posicao.Y, jogadores, qtdJogadores, dono, inimigos, quantidadeInimigos, jogadores[dono].raioBomba);
                if (roboAtivo && dono == 0) acaoRoboAtual = "Bomba explodiu";
            }
        }
        if (it->explosao.explodindo && agora - it->explosao.tempoExplodindo > 500) {
            limparExplosao(mapaAtual);
            it = bombasJogador.erase(it);
            continue;
        }
        ++it;
    }
}

void moverInimigo(int mapaAtual[TAM][TAM], Inimigo inimigos[], int indice, int quantidadeInimigos, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, Personagem jogadores[], int qtdJogadores, int dificuldade) {
    Inimigo &ini = inimigos[indice];
    if (ini.posicao.X == -1) return;
    DWORD velocidade = ini.isBoss ? 400 : 800;
    if (GetTickCount() - ini.tempoMover <= velocidade) return;

    int alvo = -1;
    int menorDist = 9999;
    for (int p = 0; p < qtdJogadores; p++) {
        if (!jogadores[p].hp || jogadores[p].posicao.X < 0) continue;
        int d = abs(ini.posicao.X - jogadores[p].posicao.X) + abs(ini.posicao.Y - jogadores[p].posicao.Y);
        if (d < menorDist) { menorDist = d; alvo = p; }
    }

    int chancePerseguir = ini.isBoss ? 100 : (dificuldade == 1 ? 50 : (dificuldade == 2 ? 75 : 0));
    if (alvo >= 0 && rand() % 100 < chancePerseguir) {
        if (ini.posicao.X < jogadores[alvo].posicao.X) ini.direcao = 1;
        else if (ini.posicao.X > jogadores[alvo].posicao.X) ini.direcao = 0;
        else if (ini.posicao.Y < jogadores[alvo].posicao.Y) ini.direcao = 3;
        else if (ini.posicao.Y > jogadores[alvo].posicao.Y) ini.direcao = 2;
    } else if (rand() % 100 < 50) ini.direcao = rand() % 4;

    int nx = ini.posicao.X, ny = ini.posicao.Y;
    if (ini.direcao == 0) nx--;
    else if (ini.direcao == 1) nx++;
    else if (ini.direcao == 2) ny--;
    else if (ini.direcao == 3) ny++;

    if (verificaCaminho(mapaAtual, nx, ny, bombasP1, bombasP2, false) && !existeInimigoNaPosicao(inimigos, quantidadeInimigos, nx, ny, indice)) {
        ini.posicao.X = nx;
        ini.posicao.Y = ny;
        
        // Toca o som de movimento do inimigo com 25% de chance se estiver perto de algum jogador ativo
        bool pertoDeJogador = false;
        for (int p = 0; p < qtdJogadores; p++) {
            if (jogadores[p].hp && jogadores[p].posicao.X != -1) {
                int d = abs(nx - jogadores[p].posicao.X) + abs(ny - jogadores[p].posicao.Y);
                if (d <= 4) {
                    pertoDeJogador = true;
                    break;
                }
            }
        }
        if (pertoDeJogador && (rand() % 100 < 25)) {
            tocarInimigo();
        }
    } else ini.direcao = rand() % 4;

    ini.tempoMover = GetTickCount();
}

void posicionarPortal(int mapaAtual[TAM][TAM]) {
    vector<Coordenadas> livres;
    for (int i = 0; i < TAM; i++) for (int j = 0; j < TAM; j++) if (mapaAtual[i][j] == 0) livres.push_back({i, j});
    if (!livres.empty()) {
        Coordenadas p = livres[rand() % livres.size()];
        mapaAtual[p.X][p.Y] = 5;
    }
}

int calcularPontosJogador(int indice) {
    return calcularPontuacaoDinamica<int>(inimigosAbatidosJogador[indice], caixasDestruidasJogador[indice], movimentosJogador[indice], bombasUsadasJogador[indice]);
}

bool existeJogadorVivo(Personagem jogadores[], int qtdJogadores) {
    for (int i = 0; i < qtdJogadores; i++) if (jogadores[i].hp) return true;
    return false;
}

bool plantarBombaJogador(int jogador, Personagem jogadores[], vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, int mapaAtual[TAM][TAM]) {
    vector<Bomba> &bombasDoJogador = (jogador == 0 ? bombasP1 : bombasP2);
    int ativas = 0;
    for (const auto &b : bombasDoJogador) if (b.bombaAtiva) ativas++;
    if (ativas >= jogadores[jogador].maxBombas) return false;
    if (existeBombaNaPosicaoTodas(bombasP1, bombasP2, jogadores[jogador].posicao.X, jogadores[jogador].posicao.Y)) return false;

    // Não permite plantar bomba dentro de caixas quebráveis (2) ou paredes (1)
    int px = jogadores[jogador].posicao.X;
    int py = jogadores[jogador].posicao.Y;
    if (mapaAtual[px][py] == 1 || mapaAtual[px][py] == 2) return false;

    Bomba novaBomba;
    novaBomba.tempobomba = GetTickCount();
    novaBomba.bombaAtiva = true;
    novaBomba.posicao = jogadores[jogador].posicao;
    bombasDoJogador.push_back(novaBomba);
    bombasUsadasJogador[jogador]++;
    bombasUsadas++;
    return true;
}

bool tentarMoverJogador(int jogador, int dx, int dy, Personagem jogadores[], int qtdJogadores, int mapaAtual[TAM][TAM], vector<Bomba> &bombasP1, vector<Bomba> &bombasP2) {
    int nx = jogadores[jogador].posicao.X + dx;
    int ny = jogadores[jogador].posicao.Y + dy;
    for (int outro = 0; outro < qtdJogadores; outro++) {
        if (outro != jogador && jogadores[outro].hp && jogadores[outro].posicao.X == nx && jogadores[outro].posicao.Y == ny) return false;
    }
    if (!verificaCaminho(mapaAtual, nx, ny, bombasP1, bombasP2, jogadores[jogador].fantasma)) return false;

    bool pegouItem = false;
    if (mapaAtual[nx][ny] >= 6 && mapaAtual[nx][ny] <= 12) {
        switch (mapaAtual[nx][ny]) {
            case 6: jogadores[jogador].raioBomba++; break;
            case 7: jogadores[jogador].maxBombas++; break;
            case 8: jogadores[jogador].velocidade++; break;
            case 9: jogadores[jogador].vidas++; break;
            case 10: jogadores[jogador].bombaRelogio = true; break;
            case 11: jogadores[jogador].invulneravel = true; break;
            case 12: jogadores[jogador].fantasma = true; break;
        }
        mapaAtual[nx][ny] = 0;
        tocarItem();
        pegouItem = true;
    }
    jogadores[jogador].posicao.X = nx;
    jogadores[jogador].posicao.Y = ny;
    movimentosJogador[jogador]++;
    movimentos++;
    if (!pegouItem) {
        tocarMovimento();
    }
    return true;
}

bool linhaLivreParaExplosao(int mapaAtual[TAM][TAM], int x1, int y1, int x2, int y2, int raio) {
    if (x1 == x2 && abs(y1-y2) <= raio) {
        int passo = (y2 > y1) ? 1 : -1;
        for (int y = y1 + passo; y != y2; y += passo) if (mapaAtual[x1][y] == 1) return false;
        return true;
    }
    if (y1 == y2 && abs(x1-x2) <= raio) {
        int passo = (x2 > x1) ? 1 : -1;
        for (int x = x1 + passo; x != x2; x += passo) if (mapaAtual[x][y1] == 1) return false;
        return true;
    }
    return false;
}

int distanciaAteInimigoMaisProximo(int x, int y, Inimigo inimigos[], int quantidadeInimigos) {
    int menor = 9999;
    for (int i = 0; i < quantidadeInimigos; i++) {
        if (inimigos[i].posicao.X == -1) continue;
        int d = abs(x - inimigos[i].posicao.X) + abs(y - inimigos[i].posicao.Y);
        if (d < menor) menor = d;
    }
    return menor;
}

int distanciaAteBombaMaisProximaEmLinha(int mapaAtual[TAM][TAM], int x, int y, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, int raio) {
    int menor = 9999;
    for (const auto &b : bombasP1) {
        if (b.bombaAtiva && linhaLivreParaExplosao(mapaAtual, b.posicao.X, b.posicao.Y, x, y, raio)) {
            int d = abs(x - b.posicao.X) + abs(y - b.posicao.Y);
            if (d < menor) menor = d;
        }
    }
    for (const auto &b : bombasP2) {
        if (b.bombaAtiva && linhaLivreParaExplosao(mapaAtual, b.posicao.X, b.posicao.Y, x, y, raio)) {
            int d = abs(x - b.posicao.X) + abs(y - b.posicao.Y);
            if (d < menor) menor = d;
        }
    }
    return menor;
}

bool existeInimigoNaPosicao(Inimigo inimigos[], int quantidadeInimigos, int x, int y, int ignorarIndice) {
    for (int i = 0; i < quantidadeInimigos; i++) {
        if (i == ignorarIndice || inimigos[i].posicao.X == -1) continue;
        if (inimigos[i].posicao.X == x && inimigos[i].posicao.Y == y) return true;
    }
    return false;
}

bool ehPowerUp(int valorMapa) {
    return valorMapa >= 6 && valorMapa <= 12;
}

bool bombaDoRoboAcertaInimigo(int mapaAtual[TAM][TAM], Coordenadas posicaoBomba, Inimigo inimigos[], int quantidadeInimigos, int raio) {
    for (int i = 0; i < quantidadeInimigos; i++) {
        if (inimigos[i].posicao.X == -1) continue;
        if (linhaLivreParaExplosao(mapaAtual, posicaoBomba.X, posicaoBomba.Y, inimigos[i].posicao.X, inimigos[i].posicao.Y, raio)) {
            return true;
        }
    }
    return false;
}

bool bombaDoRoboAcertaCaixa(int mapaAtual[TAM][TAM], Coordenadas posicaoBomba, int raio) {
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    for (int i = 0; i < 4; i++) {
        for (int passo = 1; passo <= raio; passo++) {
            int nx = posicaoBomba.X + dirs[i][0] * passo;
            int ny = posicaoBomba.Y + dirs[i][1] * passo;
            if (nx < 0 || nx >= TAM || ny < 0 || ny >= TAM) break;
            if (mapaAtual[nx][ny] == 1) break;
            if (mapaAtual[nx][ny] == 2) return true;
        }
    }
    return false;
}

bool posicaoPerigosaParaRobo(int mapaAtual[TAM][TAM], int x, int y, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, int raio, int distanciaMinimaInimigo) {
    if (x < 0 || x >= TAM || y < 0 || y >= TAM) return true;
    if (mapaAtual[x][y] == 4) return true;

    for (int i = 0; i < quantidadeInimigos; i++) {
        if (inimigos[i].posicao.X == -1) continue;
        int d = abs(x - inimigos[i].posicao.X) + abs(y - inimigos[i].posicao.Y);
        int limite = inimigos[i].isBoss ? max(2, distanciaMinimaInimigo) : distanciaMinimaInimigo;
        if (d <= limite) return true;
    }

    for (const auto &b : bombasP1) if (b.bombaAtiva && linhaLivreParaExplosao(mapaAtual, b.posicao.X, b.posicao.Y, x, y, raio)) return true;
    for (const auto &b : bombasP2) if (b.bombaAtiva && linhaLivreParaExplosao(mapaAtual, b.posicao.X, b.posicao.Y, x, y, raio)) return true;
    return false;
}

bool existeRotaDeFugaParaRobo(int mapaAtual[TAM][TAM], Personagem &robo, vector<Bomba> bombasP1, vector<Bomba> bombasP2, Inimigo inimigos[], int quantidadeInimigos) {
    Bomba simulada;
    simulada.posicao = robo.posicao;
    simulada.bombaAtiva = true;
    simulada.tempobomba = GetTickCount();
    bombasP1.push_back(simulada);

    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    bool visitado[TAM][TAM] = {};
    queue<Coordenadas> fila;
    fila.push(robo.posicao);
    visitado[robo.posicao.X][robo.posicao.Y] = true;

    while (!fila.empty()) {
        Coordenadas atual = fila.front();
        fila.pop();
        if (!posicaoPerigosaParaRobo(mapaAtual, atual.X, atual.Y, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 1)) return true;
        for (int i = 0; i < 4; i++) {
            int nx = atual.X + dirs[i][0];
            int ny = atual.Y + dirs[i][1];
            if (nx < 0 || nx >= TAM || ny < 0 || ny >= TAM || visitado[nx][ny]) continue;
            if (!verificaCaminho(mapaAtual, nx, ny, bombasP1, bombasP2, robo.fantasma)) continue;
            visitado[nx][ny] = true;
            fila.push({nx, ny});
        }
    }
    return false;
}

bool existeRotaDeFugaParaRoboNaPosicao(int mapaAtual[TAM][TAM], Personagem robo, Coordenadas posicao, vector<Bomba> bombasP1, vector<Bomba> bombasP2, Inimigo inimigos[], int quantidadeInimigos) {
    robo.posicao = posicao;
    return existeRotaDeFugaParaRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos);
}

Coordenadas escolherPosicaoAtaqueRobo(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos) {
    Coordenadas melhor = {-1, -1};
    int melhorNota = -999999;

    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (!verificaCaminho(mapaAtual, i, j, bombasP1, bombasP2, robo.fantasma)) continue;
            if (posicaoPerigosaParaRobo(mapaAtual, i, j, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 0)) continue;

            Coordenadas posicao = {i, j};
            bool acertaInimigo = bombaDoRoboAcertaInimigo(mapaAtual, posicao, inimigos, quantidadeInimigos, robo.raioBomba);
            bool acertaCaixa = bombaDoRoboAcertaCaixa(mapaAtual, posicao, robo.raioBomba);
            if (!acertaInimigo && !acertaCaixa) continue;
            if (!existeRotaDeFugaParaRoboNaPosicao(mapaAtual, robo, posicao, bombasP1, bombasP2, inimigos, quantidadeInimigos)) continue;

            int distanciaRobo = abs(robo.posicao.X - i) + abs(robo.posicao.Y - j);
            int distanciaInimigo = distanciaAteInimigoMaisProximo(i, j, inimigos, quantidadeInimigos);
            int nota = 0;
            if (acertaInimigo) nota += 1000;
            if (acertaCaixa) nota += 180;
            nota -= distanciaRobo * 25;
            nota -= min(distanciaInimigo, 10) * 4;

            if (nota > melhorNota) {
                melhorNota = nota;
                melhor = posicao;
            }
        }
    }

    return melhor;
}

Coordenadas escolherPowerUpSeguroRobo(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos) {
    Coordenadas melhor = {-1, -1};
    int melhorNota = -999999;

    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (!ehPowerUp(mapaAtual[i][j])) continue;
            if (!verificaCaminho(mapaAtual, i, j, bombasP1, bombasP2, robo.fantasma)) continue;
            if (posicaoPerigosaParaRobo(mapaAtual, i, j, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 1)) continue;

            int distancia = abs(robo.posicao.X - i) + abs(robo.posicao.Y - j);
            int nota = 500 - distancia * 20;

            if (mapaAtual[i][j] == 9) nota += 120;
            else if (mapaAtual[i][j] == 11) nota += 100;
            else if (mapaAtual[i][j] == 7 || mapaAtual[i][j] == 6) nota += 80;
            else if (mapaAtual[i][j] == 8) nota += 60;

            if (nota > melhorNota) {
                melhorNota = nota;
                melhor = {i, j};
            }
        }
    }

    return melhor;
}

int escolherMelhorDirecaoRobo(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, Coordenadas alvo, bool fugindo) {
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    int melhorDir = -1;
    int melhorNota = -999999;
    for (int i = 0; i < 4; i++) {
        int nx = robo.posicao.X + dirs[i][0];
        int ny = robo.posicao.Y + dirs[i][1];
        if (!verificaCaminho(mapaAtual, nx, ny, bombasP1, bombasP2, robo.fantasma)) continue;
        bool perigoTotal = posicaoPerigosaParaRobo(mapaAtual, nx, ny, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, fugindo ? 1 : 2);
        if (perigoTotal) continue;
        int distInimigo = distanciaAteInimigoMaisProximo(nx, ny, inimigos, quantidadeInimigos);
        int distBomba = distanciaAteBombaMaisProximaEmLinha(mapaAtual, nx, ny, bombasP1, bombasP2, robo.raioBomba);
        int distAlvo = (alvo.X == -1) ? 0 : abs(nx - alvo.X) + abs(ny - alvo.Y);
        int nota = 0;
        if (fugindo) {
            nota += min(distInimigo, 8) * 30;
            nota += min(distBomba, 8) * 20;
        } else {
            nota -= distAlvo * 12;
            nota += min(distInimigo, 6) * 15;
        }
        nota += rand() % 6;
        if (nota > melhorNota) { melhorNota = nota; melhorDir = i; }
    }
    return melhorDir;
}

int escolherDirecaoFugaRoboBFS(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos) {
    // Essa função é diferente da escolha normal de direção.
    // Quando o robô acabou de plantar uma bomba, as casas vizinhas ainda podem estar dentro do raio da explosão.
    // Então ele precisa aceitar dar 1 ou 2 passos "ainda perigosos" para chegar em uma casa realmente segura.
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};

    struct NoFuga {
        int x, y, primeiroPasso, passos;
    };

    bool visitado[TAM][TAM] = {};
    queue<NoFuga> fila;
    fila.push({robo.posicao.X, robo.posicao.Y, -1, 0});
    visitado[robo.posicao.X][robo.posicao.Y] = true;

    int melhorDir = -1;
    int melhorNota = -999999;

    while (!fila.empty()) {
        NoFuga atual = fila.front();
        fila.pop();

        if (atual.passos > 0) {
            bool seguro = !posicaoPerigosaParaRobo(mapaAtual, atual.x, atual.y, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 1);
            if (seguro) {
                int distInimigo = distanciaAteInimigoMaisProximo(atual.x, atual.y, inimigos, quantidadeInimigos);
                int distBomba = distanciaAteBombaMaisProximaEmLinha(mapaAtual, atual.x, atual.y, bombasP1, bombasP2, robo.raioBomba);
                int nota = 1000 - atual.passos * 25 + min(distInimigo, 8) * 20 + min(distBomba, 8) * 15;

                // Preferimos uma casa segura mais próxima, mas sem colar em inimigo ou bomba.
                if (nota > melhorNota) {
                    melhorNota = nota;
                    melhorDir = atual.primeiroPasso;
                }
            }
        }

        // Não precisa procurar o mapa inteiro. Em 8 passos já dá para sair do raio de uma bomba comum.
        if (atual.passos >= 8) continue;

        for (int i = 0; i < 4; i++) {
            int nx = atual.x + dirs[i][0];
            int ny = atual.y + dirs[i][1];
            if (nx < 0 || nx >= TAM || ny < 0 || ny >= TAM || visitado[nx][ny]) continue;
            if (!verificaCaminho(mapaAtual, nx, ny, bombasP1, bombasP2, robo.fantasma)) continue;

            // Durante a fuga, o robô pode atravessar momentaneamente a linha da explosão,
            // mas não deve atravessar uma casa colada em inimigo.
            bool pertoDeInimigo = false;
            for (int k = 0; k < quantidadeInimigos; k++) {
                if (inimigos[k].posicao.X == -1) continue;
                int d = abs(nx - inimigos[k].posicao.X) + abs(ny - inimigos[k].posicao.Y);
                if (d <= 1) {
                    pertoDeInimigo = true;
                    break;
                }
            }
            if (pertoDeInimigo) continue;

            visitado[nx][ny] = true;
            int primeiro = (atual.primeiroPasso == -1) ? i : atual.primeiroPasso;
            fila.push({nx, ny, primeiro, atual.passos + 1});
        }
    }

    return melhorDir;
}

int escolherDirecaoAlvoRoboBFS(int mapaAtual[TAM][TAM], Personagem &robo, const vector<Bomba> &bombasP1, const vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, Coordenadas alvo) {
    if (alvo.X == -1) return -1;

    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};

    struct NoAlvo {
        int x, y, primeiroPasso, passos;
    };

    bool visitado[TAM][TAM] = {};
    queue<NoAlvo> fila;
    fila.push({robo.posicao.X, robo.posicao.Y, -1, 0});
    visitado[robo.posicao.X][robo.posicao.Y] = true;

    // Determina se o alvo é o portal (que podemos pisar exatamente) ou outra coisa (como caixa/inimigo)
    bool ehPortal = (mapaAtual[alvo.X][alvo.Y] == 5);
    bool alvoPisavel = verificaCaminho(mapaAtual, alvo.X, alvo.Y, bombasP1, bombasP2, robo.fantasma);

    while (!fila.empty()) {
        NoAlvo atual = fila.front();
        fila.pop();

        // Se alcançou o objetivo (ou ficou adjacente no caso de caixas/inimigos)
        bool alcancou = false;
        if (ehPortal || alvoPisavel) {
            alcancou = (atual.x == alvo.X && atual.y == alvo.Y);
        } else {
            alcancou = (abs(atual.x - alvo.X) + abs(atual.y - alvo.Y) <= 1);
        }

        if (alcancou && atual.primeiroPasso != -1) {
            // Verifica se o primeiro passo é seguro (não é perigoso)
            int px = robo.posicao.X + dirs[atual.primeiroPasso][0];
            int py = robo.posicao.Y + dirs[atual.primeiroPasso][1];
            bool primeiroPassoEhAlvo = (px == alvo.X && py == alvo.Y && alvoPisavel);
            if (primeiroPassoEhAlvo || !posicaoPerigosaParaRobo(mapaAtual, px, py, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 1)) {
                return atual.primeiroPasso;
            }
        }

        // Limita a busca para evitar processamento excessivo (no console 30 passos cobre qualquer rota)
        if (atual.passos >= 30) continue;

        for (int i = 0; i < 4; i++) {
            int nx = atual.x + dirs[i][0];
            int ny = atual.y + dirs[i][1];

            if (nx < 0 || nx >= TAM || ny < 0 || ny >= TAM || visitado[nx][ny]) continue;

            // Só pode caminhar por caminhos válidos
            if (!verificaCaminho(mapaAtual, nx, ny, bombasP1, bombasP2, robo.fantasma)) continue;

            // Não deve caminhar por caminhos que sejam perigosos (com bombas ativas ou monstros colados)
            bool ehDestinoDeAtaque = (alvoPisavel && nx == alvo.X && ny == alvo.Y);
            if (!ehDestinoDeAtaque && posicaoPerigosaParaRobo(mapaAtual, nx, ny, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 1)) continue;

            visitado[nx][ny] = true;
            int primeiro = (atual.primeiroPasso == -1) ? i : atual.primeiroPasso;
            fila.push({nx, ny, primeiro, atual.passos + 1});
        }
    }

    // Se o BFS não encontrou caminho seguro, recorre ao método heurístico clássico de 1 passo
    return escolherMelhorDirecaoRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos, alvo, false);
}

void executarRobo(int mapaAtual[TAM][TAM], Personagem jogadores[], int qtdJogadores, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, Inimigo inimigos[], int quantidadeInimigos, bool portalAtivo, Coordenadas portalPos) {
    if (!roboAtivo || !jogadores[0].hp) return;
    DWORD agora = GetTickCount();
    int roboDelay = 320 - (jogadores[0].velocidade * 50);
    if (roboDelay < 70) roboDelay = 70;
    if (agora - ultimoMovimentoRobo < (DWORD)roboDelay) return;
    ultimoMovimentoRobo = agora;

    Personagem &robo = jogadores[0];
    int x = robo.posicao.X;
    int y = robo.posicao.Y;
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};

    // Se o inimigo chegar perto (distância <= 2) e não houver ameaça direta de bomba, planta a bomba e tenta fugir
    bool ameacaBomba = false;
    for (const auto &b : bombasP1) {
        if (b.bombaAtiva && linhaLivreParaExplosao(mapaAtual, b.posicao.X, b.posicao.Y, x, y, robo.raioBomba)) ameacaBomba = true;
    }
    for (const auto &b : bombasP2) {
        if (b.bombaAtiva && linhaLivreParaExplosao(mapaAtual, b.posicao.X, b.posicao.Y, x, y, robo.raioBomba)) ameacaBomba = true;
    }

    Coordenadas posicaoRobo = {x, y};
    bool inimigoNaLinhaDeBomba = bombaDoRoboAcertaInimigo(mapaAtual, posicaoRobo, inimigos, quantidadeInimigos, robo.raioBomba);
    bool caixaNaLinhaDeBomba = bombaDoRoboAcertaCaixa(mapaAtual, posicaoRobo, robo.raioBomba);

    if (!ameacaBomba && (inimigoNaLinhaDeBomba || caixaNaLinhaDeBomba) && (agora - ultimoPlantioRobo > 900)) {
        if (existeRotaDeFugaParaRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos)) {
            if (plantarBombaJogador(0, jogadores, bombasP1, bombasP2, mapaAtual)) {
                ultimoPlantioRobo = agora;
                int dirFuga = escolherDirecaoFugaRoboBFS(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos);
                if (dirFuga == -1) {
                    dirFuga = escolherMelhorDirecaoRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos, {-1, -1}, true);
                }

                if (dirFuga != -1) {
                    tentarMoverJogador(0, dirs[dirFuga][0], dirs[dirFuga][1], jogadores, qtdJogadores, mapaAtual, bombasP1, bombasP2);
                    acaoRoboAtual = inimigoNaLinhaDeBomba ? "Mirou inimigo, plantou bomba e fugiu" : "Mirou caixa, plantou bomba e fugiu";
                } else {
                    acaoRoboAtual = "Plantou bomba e procura fuga";
                }
                return;
            }
        }
    }

    bool estaEmPerigo = posicaoPerigosaParaRobo(mapaAtual, x, y, bombasP1, bombasP2, inimigos, quantidadeInimigos, robo.raioBomba, 1);
    if (estaEmPerigo) {
        int dirFuga = escolherDirecaoFugaRoboBFS(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos);
        if (dirFuga == -1) {
            dirFuga = escolherMelhorDirecaoRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos, {-1, -1}, true);
        }

        if (dirFuga != -1) {
            tentarMoverJogador(0, dirs[dirFuga][0], dirs[dirFuga][1], jogadores, qtdJogadores, mapaAtual, bombasP1, bombasP2);
            acaoRoboAtual = "Fugindo para fora do raio da bomba";
        } else {
            acaoRoboAtual = "Encurralado, procurando saida";
        }
        return;
    }

    // Se o robo ja plantou uma bomba e conseguiu sair do raio dela,
    // ele deve esperar parado. Isso evita o movimento "vai e volta" e
    // melhora a pontuacao, porque cada movimento reduz pontos.
    bool temBombaDoRoboAtiva = false;
    for (const auto &bomba : bombasP1) {
        if (bomba.bombaAtiva) {
            temBombaDoRoboAtiva = true;
            break;
        }
    }

    if (temBombaDoRoboAtiva) {
        int distanciaInimigo = distanciaAteInimigoMaisProximo(x, y, inimigos, quantidadeInimigos);

        // Se estiver fora do raio da bomba e nenhum inimigo estiver colado,
        // a melhor jogada e nao gastar movimento.
        if (distanciaInimigo > 1) {
            acaoRoboAtual = "Aguardando bomba explodir para economizar movimentos";
            return;
        }

        // Se um inimigo chegar muito perto enquanto ele espera, ele foge.
        int dirSegura = escolherMelhorDirecaoRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos, {-1, -1}, true);
        if (dirSegura != -1) {
            tentarMoverJogador(0, dirs[dirSegura][0], dirs[dirSegura][1], jogadores, qtdJogadores, mapaAtual, bombasP1, bombasP2);
            plantarBombaJogador(0, jogadores, bombasP1, bombasP2, mapaAtual);
            acaoRoboAtual = "Desviando de inimigo enquanto espera a bomba";
        } else {
            acaoRoboAtual = "Aguardando bomba explodir";
        }
        return;
    }

    bool inimigoPerto = false;
    bool caixaPerto = false;
    for (int i = 0; i < 4; i++) {
        int nx = x + dirs[i][0], ny = y + dirs[i][1];
        if (nx < 0 || nx >= TAM || ny < 0 || ny >= TAM) continue;
        for (int k = 0; k < quantidadeInimigos; k++) {
            if (inimigos[k].posicao.X == nx && inimigos[k].posicao.Y == ny && inimigos[k].posicao.X != -1) inimigoPerto = true;
        }
        if (mapaAtual[nx][ny] == 2) caixaPerto = true;
    }

    if ((inimigoPerto || caixaPerto) && agora - ultimoPlantioRobo > 900) {
        if (existeRotaDeFugaParaRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos)) {
            if (plantarBombaJogador(0, jogadores, bombasP1, bombasP2, mapaAtual)) {
                ultimoPlantioRobo = agora;
                int dirFuga = escolherDirecaoFugaRoboBFS(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos);
                if (dirFuga != -1) {
                    tentarMoverJogador(0, dirs[dirFuga][0], dirs[dirFuga][1], jogadores, qtdJogadores, mapaAtual, bombasP1, bombasP2);
                    acaoRoboAtual = "Plantou bomba e saiu do raio";
                } else {
                    acaoRoboAtual = "Plantou bomba e procura fuga";
                }
                return;
            }
        } else {
            acaoRoboAtual = "Nao plantou bomba: sem rota de fuga";
        }
    }

    Coordenadas alvo = {-1, -1};
    if (portalAtivo) {
        alvo = portalPos;
        acaoRoboAtual = "Indo para o portal";
    } else {
        alvo = escolherPowerUpSeguroRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos);
        if (alvo.X != -1) {
            acaoRoboAtual = "Indo pegar power-up seguro";
        } else {
            alvo = escolherPosicaoAtaqueRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos);
            if (alvo.X != -1) {
                bool alvoAtacaInimigo = bombaDoRoboAcertaInimigo(mapaAtual, alvo, inimigos, quantidadeInimigos, robo.raioBomba);
                acaoRoboAtual = alvoAtacaInimigo ? "Indo para posicao de ataque ao inimigo" : "Indo quebrar caixa com rota de fuga";
            }
        }
    }

    int melhorDir = escolherDirecaoAlvoRoboBFS(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos, alvo);
    if (melhorDir != -1) {
        tentarMoverJogador(0, dirs[melhorDir][0], dirs[melhorDir][1], jogadores, qtdJogadores, mapaAtual, bombasP1, bombasP2);
    } else {
        melhorDir = escolherMelhorDirecaoRobo(mapaAtual, robo, bombasP1, bombasP2, inimigos, quantidadeInimigos, alvo, true);
        if (melhorDir != -1) {
            tentarMoverJogador(0, dirs[melhorDir][0], dirs[melhorDir][1], jogadores, qtdJogadores, mapaAtual, bombasP1, bombasP2);
            acaoRoboAtual = "Passando por rota arriscada";
        } else {
            acaoRoboAtual = "Aguardando caminho seguro";
        }
    }
}

void renderizarHUD(Personagem jogadores[], int qtdJogadores, int tempoDecorrido, Inimigo inimigos[], int qtd) {
    cout << "===============================================\n";
    cout << " TEMPO: " << tempoDecorrido << "s | PAUSE: P ou ESC\n";
    for (int i = 0; i < qtdJogadores; i++) {
        int pontos = calcularPontosJogador(i);
        cout << " J" << (i + 1) << " " << playerNames[i]
             << " | PTS: " << pontos
             << " | MOV: " << movimentosJogador[i]
             << " | VIDAS: " << jogadores[i].vidas << (jogadores[i].hp ? (jogadores[i].posicao.X == -1 ? " [NO PORTAL]" : "") : " [MORTO]")
             << " | BOMBAS: " << jogadores[i].maxBombas
             << " | FOGO: " << jogadores[i].raioBomba;
        if (jogadores[i].invulneravel) cout << " [ESCUDO]";
        if (jogadores[i].fantasma) cout << " [FANTASMA]";
        if (jogadores[i].bombaRelogio) cout << " [RELOGIO]";
        cout << "\n";
    }
    if (roboAtivo) {
        cout << " ROBO: " << acaoRoboAtual;
    } else {
        cout << " CONTROLES: J1=WASD/ESPACO";
        if (qtdJogadores == 2) cout << " | J2=SETAS/ENTER";
    }

    int hpBoss = 0;
    for(int k = 0; k < qtd; k++) {
        if(inimigos[k].isBoss && inimigos[k].posicao.X != -1) {
            hpBoss = inimigos[k].vidas;
        }
    }
    if (hpBoss > 0) cout << " | BOSS HP: " << hpBoss << " 👹";
    cout << "\n===============================================\n";
}

void renderizarJogo(int mapaAtual[TAM][TAM], Personagem jogadores[], int qtdJogadores, Inimigo inimigos[], int qtd, vector<Bomba> &bombasP1, vector<Bomba> &bombasP2, vector<Bomba> &bombasBoss) {
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            bool desenhouJogador = false;
            for (int p = 0; p < qtdJogadores; p++) {
                if (jogadores[p].hp && i == jogadores[p].posicao.X && j == jogadores[p].posicao.Y) {
                    if (p == 0) cout << (jogadores[p].fantasma ? "😎" : "🤠");
                    else cout << (jogadores[p].fantasma ? "🕶️" : "🤖");
                    desenhouJogador = true;
                    break;
                }
            }

            if (!desenhouJogador) {
                bool desenhouInimigo = false;
                for (int k = 0; k < qtd; k++) {
                    if (i == inimigos[k].posicao.X && j == inimigos[k].posicao.Y && inimigos[k].posicao.X != -1) {
                        if (inimigos[k].isBoss) cout << "👹";
                        else cout << "👻";
                        desenhouInimigo = true;
                        break;
                    }
                }
                if (!desenhouInimigo) {
                    if (existeBombaNaPosicao(bombasP1, i, j)) cout << "💣";
                    else if (existeBombaNaPosicao(bombasP2, i, j)) cout << "🧨";
                    else {
                        bool ehBombaBoss = false;
                        for (const auto& bb : bombasBoss) {
                            if (bb.bombaAtiva && i == bb.posicao.X && j == bb.posicao.Y) {
                                cout << "☠️";
                                ehBombaBoss = true;
                                break;
                            }
                        }

                        if (!ehBombaBoss) {
                            switch (mapaAtual[i][j]) {
                                case 0: cout << "  "; break;
                                case 1: cout << "🧱"; break;
                                case 2: cout << "📦"; break;
                                case 4: cout << "💥"; break;
                                case 5: cout << "🚪"; break;
                                case 6: cout << "🧪"; break;
                                case 7: cout << "🧨"; break;
                                case 8: cout << "⛸️"; break;
                                case 9: cout << "💖"; break;
                                case 10: cout << "⌚"; break;
                                case 11: cout << "🛡️"; break;
                                case 12: cout << "🛹"; break;
                            }
                        }
                    }
                }
            }
        }
        cout << "\n";
    }
}

void exibirRanking() {
    limparTela();
    cout << "=========================================\n";
    cout << "             RANKING ORDENADO            \n";
    cout << "=========================================\n";
    renderizarLinhasRanking();
    cout << "=========================================\n";
    pausarParaVoltar();
}

void exibirSistemaPontuacao() {
    limparTela();
    cout << "=========================================\n";
    cout << "         CÁLCULO DE PONTUAÇÃO            \n";
    cout << "=========================================\n";
    cout << " A sua pontuação é calculada dinamicamente:\n\n";
    cout << "  +100 pontos por Inimigo Abatido\n";
    cout << "  +20  pontos por Caixa Destruída\n";
    cout << "  -1   ponto por cada Movimento feito\n";
    cout << "  -2   pontos por cada Bomba utilizada\n\n";
    cout << " Estratégia: Seja cirúrgico, gaste poucos passos\n";
    cout << " e use bombas de maneira inteligente!\n";
    cout << "=========================================\n";
    pausarParaVoltar();
}

void exibirDificuldadesEItens() {
    limparTela();
    cout << "=========================================\n";
    cout << "        DIFICULDADES E ITENS             \n";
    cout << "=========================================\n";
    cout << " DIFICULDADES:\n";
    cout << "  • Fácil   : Inimigos lentos e aleatórios.\n";
    cout << "  • Médio   : 50% de chance de Perseguição.\n";
    cout << "  • Difícil : 75% de chance de Perseguição.\n\n";
    cout << " POWER-UPS (CAIXAS):\n";
    cout << "  [🧪] Fogo    : aumenta o raio da explosao da bomba.\n";
    cout << "  [🧨] Bomba+  : permite deixar mais bombas ativas ao mesmo tempo.\n";
    cout << "  [⛸️] Patins  : aumenta a velocidade de movimento do jogador.\n";
    cout << "  [💖] Vida    : adiciona uma vida extra.\n";
    cout << "  [⌚] Relogio : faz suas bombas explodirem mais rapido.\n";
    cout << "  [🛡️] Escudo  : bloqueia um dano recebido e depois desaparece.\n";
    cout << "  [🛹] Skate   : permite atravessar caixas quebraveis.\n";
    cout << "=========================================\n";
    pausarParaVoltar();
}

void exibirComoFunciona() {
    limparTela();
    cout << "=========================================\n";
    cout << "         COMO FUNCIONA O JOGO            \n";
    cout << "=========================================\n";
    cout << " * Objetivo: eliminar todos os inimigos.\n";
    cout << " * Quando todos forem derrotados, procure\n";
    cout << "   o portal para passar de fase.\n";
    cout << " * Cuidado com os monstros e com a explosao\n";
    cout << "   das bombas.\n\n";

    cout << " MODO [1] - UM JOGADOR:\n";
    cout << "   O Jogador 1 controla o personagem sozinho.\n";
    cout << "   [W] Mover para Cima\n";
    cout << "   [S] Mover para Baixo\n";
    cout << "   [A] Mover para Esquerda\n";
    cout << "   [D] Mover para Direita\n";
    cout << "   [ESPACO] Plantar Bomba\n\n";

    cout << " MODO [2] - DOIS JOGADORES:\n";
    cout << "   Os dois jogadores jogam na mesma matriz,\n";
    cout << "   com o mesmo mapa e os mesmos inimigos.\n";
    cout << "   Jogador 1: WASD para mover e ESPACO para bomba.\n";
    cout << "   Jogador 2: SETAS para mover e ENTER para bomba.\n";
    cout << "   O ranking e calculado individualmente pelo\n";
    cout << "   nome de cada jogador.\n\n";

    cout << " MODO [3] - ROBO JOGANDO SOZINHO:\n";
    cout << "   O robo controla o personagem automaticamente.\n";
    cout << "   Ele anda, procura inimigos, planta bombas,\n";
    cout << "   foge das explosoes e tenta chegar ao portal.\n";
    cout << "   Voce pode apenas assistir e acompanhar no HUD\n";
    cout << "   o que ele esta fazendo.\n\n";

    cout << " PAUSE:\n";
    cout << "   [P] ou [ESC] pausa o jogo todo.\n";
    cout << "=========================================\n";
    pausarParaVoltar();
}

void pausarParaVoltar() {
    cout << "\nPressione qualquer tecla para voltar.";
    getch();
}

void mudarCor(int corTexto) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, corTexto);
}

void exibirMenuPrincipal() {
    // Logo em ASCII Art com cor Amarela (14)
    mudarCor(14);
    cout << R"(
  ____                  _
 |  _ \                | |
 | |_) | ___  _ __ ___ | |__   ___ _ __ _ __ ___   __ _ _ __
 |  _ < / _ \| '_ ` _ \| '_ \ / _ \ '__| '_ ` _ \ / _` | '_ \
 | |_) | (_) | | | | | | |_) |  __/ |  | | | | | | (_| | | | |
 |____/ \___/|_| |_| |_|_.__/ \___|_|  |_| |_| |_|\__,_|_| |_|
    )" << '\n';

    mudarCor(8); // Cinza escuro para as bordas
    cout << "=================================================================\n";

    mudarCor(11); // Ciano para as opções do menu
    cout << "  [1] \xF0\x9F\x8E\xAE Jogar\n";
    cout << "  [2] \xF0\x9F\x93\x96 Como Funciona o Jogo\n";
    cout << "  [3] \xF0\x9F\x93\xA6 Dificuldades e Itens\n";
    cout << "  [4] \xF0\x9F\x92\xAF Sistema de Pontuacao\n";
    cout << "  [5] \xF0\x9F\x8F\x86 Ranking Geral\n";

    mudarCor(12); // Vermelho para a opção de Sair
    cout << "  [6] \xE2\x9D\x8C Sair\n";

    mudarCor(8);
    cout << "-----------------------------------------------------------------\n";

    mudarCor(15); // Branco para o prompt de escolha
    cout << " Escolha uma opcao: ";
}

bool compararPontuacao(const Ranking &a, const Ranking &b) {
    return a.pontuacao > b.pontuacao;
}

void moverCursorParaInicio() {
    COORD coord = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void limparTela() {
    system("cls");
}

void exibirTelaVitoria() {
    limparTela();
    cout << "-------------------------------" << endl;
    cout << "         VOCE VENCEU!!         " << endl;
    cout << "-------------------------------" << endl;
    cout << "\nPressione qualquer tecla para continuar.";
    getch();
}

void exibirTelaDerrota() {
    limparTela();
    cout << "-------------------------------" << endl;
    cout << "          GAME OVER!!          " << endl;
    cout << "-------------------------------" << endl;
}

bool verificarVitoria(Inimigo inimigos[], int qtd) {
    for (int i = 0; i < qtd; i++) {
        if (inimigos[i].posicao.X != -1) return false;
    }
    return true;
}

void renderizarLinhasRanking() {
    ifstream arquivo("ranking.txt"); // CORRIGIDO: Salvamento direto na raiz para evitar erro de pasta inexistente
    if (!arquivo.is_open()) {
        cout << "   Nenhum recorde registrado ainda!\n";
        return;
    }
    string linha;
    int posicao = 1;
    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;
        stringstream ss(linha);
        string nome, pontos, bombas, movimentos, caixas, tempo, data;
        getline(ss, nome, ',');
        getline(ss, pontos, ',');
        getline(ss, bombas, ',');
        getline(ss, movimentos, ',');
        getline(ss, caixas, ',');
        getline(ss, tempo, ',');
        getline(ss, data, ',');
        printf("%2dº | %-12s | %5s pts | B:%s | M:%s | C:%s | T:%ss | %s\n", posicao, nome.c_str(), pontos.c_str(), bombas.c_str(), movimentos.c_str(), caixas.c_str(), tempo.c_str(), data.c_str());
        posicao++;
    }
    arquivo.close();
    if (posicao == 1) {
        cout << "   Nenhum recorde registrado ainda!\n";
    }
}

void exibirTelaFinal(int tempoGasto, bool venceuCampanha) {
    limparTela();
    if (venceuCampanha) {
        cout << "=========================================================================\n";
        cout << "  __      _______ _____ _______ ____  _______     __  \n";
        cout << "  \\ \\    / /_   _/ ____|__   __/ __ \\|  __ \\ \\   / /  \n";
        cout << "   \\ \\  / /  | || |       | | | |  | | |__) \\ \\_/ /   \n";
        cout << "    \\ \\/ /   | || |       | | | |  | |  _  / \\   /    \n";
        cout << "     \\  /   _| || |____   | | | |__| | | \\ \\  | |     \n";
        cout << "      \\/   |_____\\_____|  |_|  \\____/|_|  \\_\\ |_|     \n";
        cout << "=========================================================================\n";
        cout << "                       ___________                                       \n";
        cout << "                      '._==_==_==_.'                                     \n";
        cout << R"(                      .-\:      /-.
)";
        cout << "                     | (|:.     |) |                                     \n";
        cout << "                      '-|:.     |-'                                      \n";
        cout << "                        \\::.    /                                        \n";
        cout << "                         '::. .'                                         \n";
        cout << "                           ) (                                           \n";
        cout << "                         _.' '._                                         \n";
        cout << "                        `\"\"\"\"\"\"\"`                                        \n";
        cout << "=========================================================================\n";
        cout << "                  PARABENS! VOCE VENCEU A CAMPANHA!                      \n";
        cout << "=========================================================================\n";
    } else {
        cout << "=========================================================================\n";
        cout << "    _____          __  __ ______    ______      ________ _____   \n";
        cout << "   / ____|   /\\   |  \\/  |  ____|  / __ \\ \\    / /  ____|  __ \\  \n";
        cout << "  | |  __   /  \\  | \\  / | |__    | |  | \\ \\  / /| |__  | |__) |\n";
        cout << "  | | |_ | / /\\ \\ | |\\/| |  __|   | |  | |\\ \\/ / |  __| |  _  / \n";
        cout << "  | |__| |/ ____ \\| |  | | |____  | |__| | \\  /  | |____| | \\ \\  \n";
        cout << "   \\_____/_/    \\_\\_|  |_|______|  \\____/   \\/   |______|_|  \\_\\\n";
        cout << "=========================================================================\n";
        cout << "                          .---.                                          \n";
        cout << "                         /     \\                                         \n";
        cout << "                        | R.I.P |                                        \n";
        cout << "                        |       |                                        \n";
        cout << "                        |       |                                        \n";
        cout << "                     \\|       |//                                      \n";
        cout << "                    ^^^^^^^^^^^^^^^                                      \n";
        cout << "=========================================================================\n";
        cout << "                     GAME OVER - TENTE NOVAMENTE!                        \n";
        cout << "=========================================================================\n";
    }
    cout << " Tempo total da campanha: " << tempoGasto << " segundos\n";
    for (int i = 0; i < quantidadeJogadoresCampanha; i++) {
        int pontos = calcularPontosJogador(i);
        cout << "-------------------------------------------------------------------------\n";
        cout << " Jogador " << (i + 1) << ": " << playerNames[i] << "\n";
        cout << " Pontuacao final   : " << pontos << " pontos\n";
        cout << " Movimentos        : " << movimentosJogador[i] << "\n";
        cout << " Bombas usadas     : " << bombasUsadasJogador[i] << "\n";
        cout << " Inimigos abatidos : " << inimigosAbatidosJogador[i] << "\n";
        cout << " Caixas destruidas : " << caixasDestruidasJogador[i] << "\n";
    }
    cout << "=========================================================================\n";
    cout << "\nPressione qualquer tecla para voltar ao menu.";
    getch();
}

void atualizarRanking(const string &playerName, int totalPontos, int tempoGasto, int jogador) {
    vector<Ranking> listaRanking;
    ifstream arquivoLeitura("ranking.txt");
    if (arquivoLeitura.is_open()) {
        string linha;
        while (getline(arquivoLeitura, linha)) {
            if (linha.empty()) continue;
            stringstream ss(linha);
            string nomeToken, pontosToken, bombasToken, movimentosToken, caixasToken, tempoToken, dataToken;
            getline(ss, nomeToken, ',');
            getline(ss, pontosToken, ',');
            getline(ss, bombasToken, ',');
            getline(ss, movimentosToken, ',');
            getline(ss, caixasToken, ',');
            getline(ss, tempoToken, ',');
            getline(ss, dataToken, ',');
            if (!nomeToken.empty() && !pontosToken.empty()) {
                Ranking reg;
                reg.playerName = nomeToken;
                reg.pontuacao = stoi(pontosToken);
                reg.bombas = bombasToken.empty() ? 0 : stoi(bombasToken);
                reg.movimentos = movimentosToken.empty() ? 0 : stoi(movimentosToken);
                reg.caixas = caixasToken.empty() ? 0 : stoi(caixasToken);
                reg.tempo = tempoToken.empty() ? 0 : stoi(tempoToken);
                reg.data = dataToken;
                listaRanking.push_back(reg);
            }
        }
        arquivoLeitura.close();
    }

    time_t now = time(0);
    tm *ltm = localtime(&now);
    char dataStr[20];
    sprintf(dataStr, "%04d-%02d-%02d %02d:%02d", 1900+ltm->tm_year, 1+ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min);

    Ranking novoRegistro;
    novoRegistro.playerName = playerName;
    novoRegistro.pontuacao = (totalPontos < 0 ? 0 : totalPontos);
    novoRegistro.bombas = bombasUsadasJogador[jogador];
    novoRegistro.movimentos = movimentosJogador[jogador];
    novoRegistro.caixas = caixasDestruidasJogador[jogador];
    novoRegistro.tempo = tempoGasto;
    novoRegistro.data = dataStr;
    listaRanking.push_back(novoRegistro);

    sort(listaRanking.begin(), listaRanking.end(), compararPontuacao);

    ofstream arquivoEscrita("ranking.txt", ios::trunc);
    if (arquivoEscrita.is_open()) {
        for (const auto &reg : listaRanking) {
            arquivoEscrita << reg.playerName << "," << reg.pontuacao << "," << reg.bombas << "," << reg.movimentos << "," << reg.caixas << "," << reg.tempo << "," << reg.data << "\n";
        }
        arquivoEscrita.close();
    }
}

bool posicaoProtegida(int linha, int coluna) {
    // Area protegida do Jogador 1: canto superior esquerdo.
    if (linha == 1 && coluna == 1) return true;
    if (linha == 1 && coluna == 2) return true;
    if (linha == 1 && coluna == 3) return true;
    if (linha == 2 && coluna == 1) return true;
    if (linha == 3 && coluna == 1) return true;

    // Area protegida do Jogador 2: canto superior direito.
    // Ele nasce em {1,13} e tem uma saida pela esquerda e outra para baixo.
    if (quantidadeJogadoresCampanha == 2) {
        if (linha == 1 && coluna == 13) return true;
        if (linha == 1 && coluna == 12) return true;
        if (linha == 1 && coluna == 11) return true;
        if (linha == 2 && coluna == 13) return true;
        if (linha == 3 && coluna == 13) return true;
    }
    return false;
}

void limparAreasProtegidasJogadores(int mapaAtual[TAM][TAM]) {
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (posicaoProtegida(i, j) && mapaAtual[i][j] != 1) {
                mapaAtual[i][j] = 0;
            }
        }
    }
}

bool todosJogadoresPassaramPortal(bool jogadoresNoPortal[], Personagem jogadores[], int qtdJogadores) {
    bool alguemVivo = false;

    for (int i = 0; i < qtdJogadores; i++) {
        if (jogadores[i].hp) {
            alguemVivo = true;
            if (!jogadoresNoPortal[i]) return false;
        }
    }

    return alguemVivo;
}

void gerarCaixasAleatorias(int mapaAtual[TAM][TAM], int chanceCaixa) {
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (mapaAtual[i][j] == 0 && !posicaoProtegida(i, j)) {
                if (rand() % 100 < chanceCaixa) {
                    mapaAtual[i][j] = 2;
                }
            }
        }
    }
}


int escolherQuantidadeJogadores() {
    while (true) {
        limparTela();
        cout << "=========================================\n";
        cout << "              MODO DE JOGO               \n";
        cout << "=========================================\n";
        cout << " [1] Um jogador\n";
        cout << "     Jogador 1 usa WASD e ESPACO.\n\n";
        cout << " [2] Dois jogadores\n";
        cout << "     Jogador 1 usa WASD e ESPACO.\n";
        cout << "     Jogador 2 usa SETAS e ENTER.\n";
        cout << "     Cada jogador informa seu nome e tem\n";
        cout << "     ranking individual.\n\n";
        cout << " [3] Robo jogando sozinho\n";
        cout << "     O robo faz tudo automaticamente:\n";
        cout << "     anda, planta bombas, foge e busca o portal.\n";
        cout << "     Voce acompanha a acao dele na tela.\n";
        cout << "-----------------------------------------\n";
        cout << " Escolha uma opcao: ";
        char opcao = getch();
        if (opcao == '1') return 1;
        if (opcao == '2') return 2;
        if (opcao == '3') return 3;
    }
}

int escolherDificuldade() {
    while (true) {
        limparTela();
        cout << "=========================================\n";
        cout << "          ESCOLHA A DIFICULDADE          \n";
        cout << "=========================================\n";
        cout << " [1] Fácil\n";
        cout << " [2] Médio\n";
        cout << " [3] Difícil\n";
        cout << "-----------------------------------------\n";
        cout << " Escolha uma opção: ";

        char opcao = getch();

        if (opcao == '1') return 0;
        if (opcao == '2') return 1;
        if (opcao == '3') return 2;
    }
}

void executarJogo() {
    SetConsoleOutputCP(CP_UTF8);
    srand(time(NULL));

    inicializarAudio();

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);

    bool tocarMusicaMenu = true; // Controle da música do menu

    while(true) {
        if (tocarMusicaMenu) {
            tocarIntro();
            tocarMusicaMenu = false;
        }

        limparTela();
        exibirMenuPrincipal();

        char opcaoMenu = getch();

        if (opcaoMenu == '1') {
            pararIntro(); // Desliga a música para começar o jogo

            modoJogoCampanha = escolherQuantidadeJogadores();
            roboAtivo = (modoJogoCampanha == 3);
            quantidadeJogadoresCampanha = (modoJogoCampanha == 2 ? 2 : 1);
            int dificuldade = escolherDificuldade();
            playerNames[0] = "";
            playerNames[1] = "";

            // Reinicia os contadores apenas no começo da campanha, não a cada fase.
            movimentos = 0;
            bombasUsadas = 0;
            inimigosAbatidos = 0;
            caixasDestruidas = 0;
            tempoCampanhaGasto = 0;
            for (int i = 0; i < 2; i++) {
                movimentosJogador[i] = 0;
                bombasUsadasJogador[i] = 0;
                inimigosAbatidosJogador[i] = 0;
                caixasDestruidasJogador[i] = 0;
            }

            for (int fase = 0; fase < quantidadeFases; fase++) {
                bool venceu = iniciarJogo(dificuldade, fase);
                if (!venceu) break;
            }
            tocarMusicaMenu = true; // Reativa a música ao retornar para o menu
        }
        else if (opcaoMenu == '2') exibirComoFunciona();
        else if (opcaoMenu == '3') exibirDificuldadesEItens();
        else if (opcaoMenu == '4') exibirSistemaPontuacao();
        else if (opcaoMenu == '5') exibirRanking();
        else if (opcaoMenu == '6') {
            limparTela();
            cout << "Obrigado por jogar! Saindo...\n";
            fecharAudio();
            break;
        }
    }
    fecharAudio();
}

int main() {
    executarJogo();
    return 0;
}
