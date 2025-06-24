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
int envia_com_ack(int soquete, char* iface, unsigned char* dados, int tamanho, unsigned char tipo, unsigned char *seq) {
    int tentativa = 0, max_tentativas = 5, timeout = 500;
    Frame f;
    unsigned char* msg = monta_mensagem(tamanho, *seq, tipo, dados);            // Monta o frame a ser enviado
    unsigned char buffer[2048];

    // Tenta enviar a mensagem até atingir o número máximo de tentativas
    while (tentativa < max_tentativas) {
        envia_mensagem(soquete, msg, tamanho + 4, iface);                       // Envia a mensagem (dados + cabeçalho)

        // Aguarda resposta (ACK ou NACK)
        if (recebe_mensagem(soquete, timeout, &f, buffer, sizeof(buffer)) > 0) {
            if (f.sequencia == *seq) {          // Se a sequência bate, verifica o tipo da resposta
                if (f.tipo == 2) {
                    free(msg);
                    return 2;                   // ACK com confirmação (ACK + OK)
                } 
                
                else if (f.tipo == 0) {
                    free(msg);
                    return 1;                   // ACK simples
                } 
                
                else if (f.tipo == 1) {
                    printf("Recebido NACK. Reenviando...\n");           // NACK recebido: deve reenviar
                }
            }
        } 
        
        else {
            
            printf("Timeout. Reenviando com recuo...\n");               // Timeout: nenhum frame foi recebido, duplica o tempo de espera (exponencial backoff)
        }

        tentativa++;
        timeout *= 2;           // Backoff exponencial no tempo de espera
    }

    // Falhou após todas as tentativas
    free(msg);
    return 0;
}

