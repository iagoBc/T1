#include "enviar.h"

// Monta um frame para envio com os campos de controle (cabeçalho), dados e checksum
unsigned char* monta_mensagem(short tamanho, unsigned char sequencia, unsigned char tipo, unsigned char *dados) {
    if (tamanho > 127) exit(1);             // Verifica se o tamanho excede 127 bytes (limite de 7 bits para o campo de tamanho)
    short total = 4 + tamanho;              // Tamanho total do frame: 4 bytes de cabeçalho + dados
    unsigned char *mensagem = malloc(total);

    unsigned char seq_msb = (sequencia >> 4) & 0x01;                    // Extrai o bit mais significativo (MSB) da sequência (bit 4) → 1 bit
    unsigned char seq_lsb = sequencia & 0x0F;                           // Extrai os 4 bits menos significativos da sequência (bits 0 a 3) → 4 bits
    unsigned char campo1 = ((tamanho & 0x7F) << 1) | seq_msb;           // campo1: [7 bits de tamanho << 1] | [1 bit do MSB da sequência]
    unsigned char campo2 = (seq_lsb << 4) | (tipo & 0x0F);              // campo2: [4 bits de LSB da sequência << 4] | [4 bits do tipo da mensagem]

    unsigned char checksum = calcula_checksum(campo1, campo2, dados, tamanho);              // Calcula o checksum: XOR entre campo1, campo2 e todos os bytes de dados  

    // Preenche os 4 primeiros bytes do frame com os campos de controle
    mensagem[0] = 0x7E;                 // Delimitador de início do frame
    mensagem[1] = campo1;               // Campo 1: tamanho + MSB da sequência
    mensagem[2] = campo2;               // Campo 2: LSB da sequência + tipo da mensagem
    mensagem[3] = checksum;             // Campo de verificação de integridade

    memcpy(&mensagem[4], dados, tamanho);               // Copia os dados (payload) a partir da posição 4

    return mensagem;                // Retorna o ponteiro para o frame montado
}

// Envia uma mensagem via raw socket para a interface de rede especificada
void envia_mensagem(int soquete, unsigned char* dados, int tamanho_dados, char* nome_interface_rede) {
    struct ifreq if_idx, if_mac;                 // Estruturas para armazenar índice e endereço MAC da interface
    struct sockaddr_ll endereco;                 // Estrutura de endereço da camada de enlace
    unsigned char frame[1514];                   // Buffer para armazenar o quadro Ethernet (máx. 1514 bytes)

    //  Obtenção do índice da interface
    memset(&if_idx, 0, sizeof(if_idx));
    strncpy(if_idx.ifr_name, nome_interface_rede, IFNAMSIZ - 1);
    ioctl(soquete, SIOCGIFINDEX, &if_idx);              // ioctl para buscar o índice da interface (necessário para envio)

    // Obtenção do MAC da interface
    memset(&if_mac, 0, sizeof(if_mac));
    strncpy(if_mac.ifr_name, nome_interface_rede, IFNAMSIZ - 1);
    ioctl(soquete, SIOCGIFHWADDR, &if_mac);             // ioctl para buscar o endereço MAC da interface

    // Montagem do quadro Ethernet
    memset(frame, 0, sizeof(frame));                    // Zera todo o buffer do frame
    memset(frame, 0xFF, 6);                             // Endereço MAC de destino: broadcast (FF:FF:FF:FF:FF:FF)
    memcpy(frame + 6, if_mac.ifr_hwaddr.sa_data, 6);    // Copia o MAC de origem (6 bytes)

    // Define o campo "EtherType" como valor personalizado 0x88B5 (dois bytes)
    frame[12] = 0x88;
    frame[13] = 0xB5;

    memcpy(frame + 14, dados, tamanho_dados);           // Copia os dados do payload para a partir do byte 14 do frame

    // Prepara o endereço de envio na estrutura sockaddr_ll
    endereco.sll_ifindex = if_idx.ifr_ifindex;          // Índice da interface
    endereco.sll_halen = ETH_ALEN;                      // Tamanho do endereço MAC
    endereco.sll_family = AF_PACKET;                    // Família para sockets de camada de enlace
    memset(endereco.sll_addr, 0xFF, 6);                 // Endereço MAC de destino: broadcast

    sendto(soquete, frame, tamanho_dados + 14, 0, (struct sockaddr*)&endereco, sizeof(endereco));           // Envia o frame Ethernet 
}

// Envia uma mensagem via socket RAW e espera por um ACK correspondente
int envia_com_ack(int soquete, char* iface, unsigned char* dados, int tamanho, unsigned char tipo, unsigned char* seq) {
    int tentativa = 0;                      // Contador de tentativas realizadas
    int max_tentativas = 5;                 // Número máximo de tentativas antes de desistir
    int timeout = 500;                      // Timeout inicial em milissegundos (500 ms)
    Frame f;                                // Estrutura para armazenar o frame recebido
    unsigned char* msg = monta_mensagem(tamanho, *seq, tipo, dados);            // Monta o frame a ser enviado
    unsigned char buffer[2048];             // Buffer para receber mensagens

    // Loop principal de tentativas
    while (tentativa < max_tentativas) {
        envia_mensagem(soquete, msg, tamanho + 4, iface);               // Envia a mensagem montada para a interface especificada

        // Espera uma resposta com o timeout definido
        if (recebe_mensagem(soquete, timeout, &f, buffer, sizeof(buffer)) > 0) {
            if (f.sequencia == *seq) {          // Verifica se a sequência da resposta corresponde à sequência enviada
                if (f.tipo >= 6 && f.tipo <= 8) {           // Se o tipo da mensagem recebida está entre 6 e 8 tesouro encontrado
                    free(msg);
                    caminho = malloc(f.tamanho_dados + 9);          // Aloca memória para armazenar o caminho completo do arquivo

                    // Copia os dados do nome do arquivo recebido para uma string
                    char nome_arq[f.tamanho_dados + 1];   
                    for (int i = 0; i < f.tamanho_dados; i++) {
                        nome_arq[i] = (char)f.dados[i];
                    }
                    nome_arq[f.tamanho_dados] = '\0';           // Finaliza a string com '\0'

                    // Cria o caminho completo concatenando "objetos/" com o nome do arquivo
                    snprintf(caminho, f.tamanho_dados + 9, "objetos/%s", nome_arq);

                    // Inicia o processo de baixar o arquivo usando o socket e interface
                    if(baixa_arquivo(soquete, iface, &f, seq) == 15){
                        perror("ERRO AO BAIXAR ARQUIVO\n");
                        return 15;
                    }

                    return 2;               // Retorna 2 para realizar o movimento    
                }

                else if (f.tipo == 1) {             // Se recebeu um NACK (tipo == 1), sinaliza para reenviar a mensagem
                    printf("Recebido NACK. Reenviando...\n");
                }

                else if (f.tipo == 14) {
                    return 14;
                }

                else if (f.tipo == 15) {
                    return 15;
                }

                else {          // Para outros tipos de resposta (geralmente ACK), incrementa a sequência e retorna o tipo
                    *seq = (*seq + 1) % 32;             // Incrementa sequência com wrap-around (0-31)
                    return f.tipo;
                }
            }
        } 
        
        else {
            printf("Timeout. Reenviando com recuo...\n");               // Se o tempo limite expirou sem receber resposta válida, tenta reenviar
        }

        tentativa++;            // Incrementa o contador de tentativas
        timeout *= 2;           // Timeout exponencial: dobra o tempo de espera a cada tentativa
    }

    free(msg);                  // Libera memória da mensagem caso todas tentativas falhem
    return 15;                  // Retorna 15 indicando falha após todas as tentativas
}
