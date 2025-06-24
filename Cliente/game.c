#include "raw_sockets.h"                                                                        // Biblioteca para comunicação usando sockets RAW                                                                  // Lida com a lógica do mapa e movimentação do avatar

int tamanho_arq;                                                                                
char* caminho;                                                                                  // Caminho(nome) do arquivo
FILE* arquivo;

int main() {
    Mapa mapa;                                                                                  // Estrutura que representa o mapa e a posição atual

    char* interface = "enp4s0";                                                                  // Interface de rede a ser usada (pode variar conforme o ambiente)
    int soquete = cria_raw_socket(interface);                                                   // Cria um socket RAW na interface especificada
    unsigned char sequencia = 0;                                                                // Número de sequência das mensagens enviadas

    printf("Esperando a inicialização do servidor...");                                         // Espera o servidor iniciar e envia ACK com a posição inicial
    Frame f;
    unsigned char buffer[2048];                                                                 // Buffer para armazenar dados recebidos
    espera_servidor(&mapa, soquete, interface, 3000, &f, buffer, sizeof(buffer));
    inicializa_mapa(&mapa);                                                                     // Inicializa o mapa com a posição inicial recebida

    int game = 0;
    char move;
    unsigned char comando = 0x00;                                                               // Comando a ser enviado

    while (!game) {
        if (move != '\n') print_mapa_cliente(&mapa, comando);                                   // Imprime o mapa se o usuário digitou algo diferente de '\n'

        move = getchar();                                                                       // Captura entrada do usuário

        if (move == 'w') comando = 0x0B;                                                        // cima
        else if (move == 's') comando = 0x0C;                                                   // baixo
        else if (move == 'a') comando = 0x0D;                                                   // esquerda
        else if (move == 'd') comando = 0x0A;                                                   // direita
        else if (move == 'q') {                                                                 // Encerrar o jogo
            game = 1;
            break;
        } else {
            continue;                                                                           // Ignora qualquer outro caractere
        }

        if (comando >= 0x0A && comando <= 0x0D) {                                               // Envia comando de movimento ao servidor
            printf("\033[H\033[J"); 
            unsigned char dados[1] = {comando};                                                 // Dados do comando
            int retorno = envia_com_ack(soquete, interface, dados, 1, comando, &sequencia);

            // Verifica o retorno:
            if (retorno == 2){
                printf("\n");
                movimenta_peca(&mapa, comando);                                                 // Move o avatar no mapa se comando for válido
            } else if (retorno == 0){
                printf("ACK recebido, mas movimento inválido (fora do mapa).\n");
            } else if(retorno == 15){
                printf("\nERRO ENCONTADO, Problema ao receber arquivo");
                print_game_over_ascii();
                break;
            } else if(retorno == 14){
                printf("\033[H\033[J"); 
                print_game_over_ascii();
                printf("\n\n TODOS OS TESOUROS ENCONTRADOS\n");

                break;
            }                      
        }
    }

    close(soquete);                                                                             // Fecha o socket ao encerrar
    return 0;
}

