#ifndef __SERVIDOR__
#define __SERVIDOR__


#include <unistd.h>
#include <time.h>

#include "raw_sockets.h"
#include "mapa_servidor.h"
#include "libArquivo.h"

// Inicializa servidor com 8 tesouros em posiçoes diferentes e posição inicial aleatoria
void inicializa_servidor(Mapa *m);

// Envia para o cliente posição inicial do personagem
void envia_posi_inicio(Mapa *m, int soquete, char *interface, unsigned char* seq);

// Retorna o tipo de ack especifico para o movimento
unsigned char trata_comando(Frame *f, Mapa* mapa);

// Retorna 1 se na posição atual do avatar tem um tesouro
int encontrou_tesouro(Mapa *m);

// Trata o evento de encontro com um tesouro: envia nome, tipo, tamanho e conteúdo do arquivo associado
void trata_encontro(Mapa *m, int soquete, char *interface, unsigned char *sequencia);

// Retorna 1 caso todos os tesouros ja terem sido encontrados
int todos_tesouros_encontrados(Mapa *m);

#endif