#include "servidor.h"

int main() {
    char* interface = "eno1";              // Nome da interface de rede usada (ex: "eth0", "veth0", etc.)
    int soquete = cria_raw_socket(interface);           // Cria o socket RAW para enviar/receber quadros Ethernet
    Frame f;
    unsigned char buffer[2048];             // Buffer para armazenar dados recebidos
    unsigned char sequencia = 0;            // ✅ Sequência de envio do servidor (controla ordenação dos pacotes)
    int tentativas = 0;
    int timeout = 3000;

    Mapa mapa;
    inicializa_servidor(&mapa);         // Inicializa o estado do jogo/servidor
    envia_posi_inicio(&mapa, soquete, interface, &sequencia);           // Envia posição inicial do jogador com sequência 0

    while (1) {         // Loop principal do servidor
        print_mapa_servidor(&mapa);         // Mostra no terminal o estado atual do mapa
        int fim_programa = 0;

        int status = recebe_mensagem(soquete, timeout, &f, buffer, sizeof(buffer));            // Espera receber um comando do cliente (com timeout de 3 segundos)
        
        if (status > 0) {
            printf("Comando recebido (tipo %d, seq %d)\n", f.tipo, f.sequencia);
            unsigned char ack_type;
            unsigned char* resposta;
            tentativas = 0;
            timeout = 3000;

            if (ack_type == 14 || ack_type == 15)
                fim_programa = 1;

            ack_type = trata_comando(&f, &mapa); // Processa o comando recebido e obtém o tipo de resposta (ACK, NACK, etc.)

            if (ack_type == 0x02) {             // 0x02 indica que o movimento é válido
                movimenta_peca(&mapa, f.tipo);  // Move o jogador no mapa conforme o comando
                printf("Movimento executado. Nova posição: (%d, %d)\n", mapa.linha, mapa.coluna);
                
                if (encontrou_tesouro(&mapa)) {         // Verifica se o jogador encontrou um tesouro
                    printf("Tesouro encontrado!\n");

                    // ✅ Se encontrar tesouro, envia nome do arquivo usando a sequência do cliente
                    trata_encontro(&mapa, soquete, interface, &f.sequencia);
                }

                mapa.visitas[mapa.linha][mapa.coluna] = 1;          // Marca a nova posição no mapa como visitada
            } 

            else if (ack_type == 0x0F){
                printf("Erro encontrado");
                ack_type = 0x0F;
                break;
            }
            
            else {
                printf("Movimento inválido. Enviando apenas ACK.\n");
            }

            if (todos_tesouros_encontrados(&mapa)) // Se todos os tesouros ja tiverem sido encontrados
                ack_type = 0x0E;

            // Envia um ACK/NACK genérico de resposta à ação do cliente
            resposta = monta_mensagem(0, f.sequencia, ack_type, NULL);
            envia_mensagem(soquete, resposta, 4, interface);
            free(resposta);
            
            if (fim_programa)
                break;
            
        } 
        
        else {
            printf("⏳ Timeout aguardando comando... Tempo de espera %d ms\n", timeout);       // Timeout: nenhum comando recebido dentro do período esperado
            timeout *= 2;
            tentativas++;
            if (tentativas >= 5)
                break;

        }
    }

    close(soquete);         // Fecha o socket
    return 0;
}
