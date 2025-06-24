#include "mapa_cliente.h"
                                                       
void inicializa_mapa(Mapa* m) {                                             // Inicializa todas as posições do mapa como "não visitadas"
    for (int i = 7; i >= 0; i--)                                            // Percorre as linhas de cima para baixo (linha 7 até 0)
        for (int j = 0; j < 8; j++)                                         // Percorre as colunas da esquerda para a direita (0 até 7)
            m->visitas[i][j] = 0;                                           // Marca cada célula como não visitada (0)
}

                                                            
void movimenta_peca(Mapa* m, unsigned char comando) {                       // Move o avatar no mapa com base no comando recebido
    if (comando == 0x0A && m->coluna < 7) m->coluna++;                      // Comando 0x0A: mover para a direita (incrementa a coluna se não estiver no limite)
    else if (comando == 0x0B && m->linha < 7) m->linha++;                   // Comando 0x0B: mover para cima (incrementa a linha se não estiver no limite inferior)
    else if (comando == 0x0C && m->linha > 0) m->linha--;                   // Comando 0x0C: mover para baixo (decrementa a linha se não estiver no limite superior)
    else if (comando == 0x0D && m->coluna > 0) m->coluna--;                 // Comando 0x0D: mover para a esquerda (decrementa a coluna se não estiver na borda esquerda)

    m->visitas[m->linha][m->coluna] = 1;                                    // Marca a nova posição como visitada
}

void print_mapa_cliente(Mapa* m, unsigned char comando){                    // Imprime o estado atual do mapa no terminal
    printf("\nMapa (comando %02X):\n", comando);                            // Mostra o comando atual

    for (int i = 7; i >= 0; i--) {                                          // Imprime o mapa de cima para baixo
        printf("    ");
        for (int j = 0; j < 8; j++) {
            if (i == m->linha && j == m->coluna) printf(" (P) ");           // Se a posição atual é a do jogador, imprime "P"
            else printf(" ( ) ");                                           // Caso contrário, imprime espaço vazio
        }
        printf("\n");
    }
}

#include <stdio.h>

void print_game_over_ascii() {
    printf("\n\n");
    printf("  ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ \n");
    printf(" ██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗\n");
    printf(" ██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝\n");
    printf(" ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║██║   ██║██╔══╝  ██╔══██╗\n");
    printf(" ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝╚██████╔╝███████╗██║  ██║\n");
    printf("  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═════╝ ╚══════╝╚═╝  ╚═╝\n");
    printf("\n\n");
}

