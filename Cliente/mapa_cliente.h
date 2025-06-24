#ifndef MAPA_H
#define MAPA_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {                                                            // Estrutura para armazenar as posições do mapa
    int linha;
    int coluna;
    int visitas[8][8];                                                      // Matriz 8x8 para controlar as posições visitadas
} Mapa;

// Imprime o estado atual do mapa no terminal
void print_mapa_cliente(Mapa* mapa, unsigned char comando);                 

// Inicializa todas as posições do mapa como "não visitadas"
void inicializa_mapa(Mapa* m);                                              

// Move o avatar no mapa com base no comando recebido
void movimenta_peca(Mapa* m, unsigned char comando);  

// Printa game over
void print_game_over_ascii();

#endif