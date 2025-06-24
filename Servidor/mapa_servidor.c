#include "mapa_servidor.h"

// Imprime o estado atual do mapa no terminal
void print_mapa_servidor(Mapa* mapa) {
    printf("\n");

    for (int i = 7; i >= 0; i--) {              // Percorre o mapa de cima para baixo (linha 7 a 0)
        printf("    ");
        for (int k = 0; k < 8; k++) {           // Colunas da esquerda para a direita
            int tem_tesouro = 0;

            for (int t = 0; t < 8; t++) {       // Verifica se há tesouro nessa posição
                if (mapa->tesouro[t].linhaT == i && mapa->tesouro[t].colunaT == k) {
                    tem_tesouro = 1;
                    break;
                }
            }

            int eh_personagem = (i == mapa->linha && k == mapa->coluna);        // Verifica se essa posição é onde o personagem está

            // Define o que mostrar para cada célula:
            if (tem_tesouro && eh_personagem) printf(" (@) ");                  // Personagem em cima do tesouro
            
            else if (tem_tesouro && mapa->visitas[i][k]) printf(" (#) ");       // Tesouro já visitado
            
            else if (tem_tesouro) printf(" (X) ");                              // Tesouro não visitado
            
            else if (eh_personagem) printf(" (p) ");                            // Personagem 
            
            else if (mapa->visitas[i][k]) printf(" (.) ");                      // Posição já visitada
            
            else printf(" ( ) ");                                               // Posição vazia
            
        }
        printf("\n");
    }                                                       // Marca a origem no canto inferior esquerdo
    printf("\n");
}

// Move a peça de acordo com o comando recebido
void movimenta_peca(Mapa* m, unsigned char comando) {
    if (comando == 0x0A && m->coluna < 7) m->coluna++;              // Move para a direita, se ainda não está na borda direita

    else if (comando == 0x0B && m->linha < 7) m->linha++;           // Move para cima, se ainda não está na borda superior

    else if (comando == 0x0C && m->linha > 0) m->linha--;           // Move para baixo, se ainda não está na borda inferior

    else if (comando == 0x0D && m->coluna > 0) m->coluna--;         // Move para a esquerda, se ainda não está na borda esquerda
}
