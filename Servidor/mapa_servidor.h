#ifndef __MAPA_H__
#define __MAPA_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int linhaT;
    int colunaT;
}Tesouros;

// Estrutura para armazenar as posições do mapa
typedef struct {
    int linha;
    int coluna;
    int visitas[8][8];  // Matriz 8x8 para controlar as posições visitadas
    Tesouros tesouro[8]; // 8 tesouros armazenados em posiçoes
} Mapa;

// Imprime o estado atual do mapa no terminal
void print_mapa_servidor(Mapa* mapa);

// Move a peça de acordo com o comando recebido
void movimenta_peca(Mapa* m, unsigned char comando);

#endif