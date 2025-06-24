#include "raw_sockets.h"

// Cria um socket raw para interceptar e enviar pacotes diretamente na camada de enlace (Ethernet)
int cria_raw_socket(char* nome_interface_rede) {
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));                // Cria um socket do tipo RAW que captura todos os protocolos Ethernet (ETH_P_ALL)
    if (soquete == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    int ifindex = if_nametoindex(nome_interface_rede);              // Obtém o índice da interface de rede a partir de seu nome

    struct sockaddr_ll endereco = {0};                // Prepara a estrutura de endereço de baixo nível (sockaddr_ll) para fazer o bind do socket
    endereco.sll_family = AF_PACKET;                  // Família de protocolos de baixo nível (Ethernet)
    endereco.sll_protocol = htons(ETH_P_ALL);         // Captura todos os protocolos Ethernet
    endereco.sll_ifindex = ifindex;                   // Índice da interface de rede

    // Associa (bind) o socket à interface especificada
    if (bind(soquete, (struct sockaddr*)&endereco, sizeof(endereco)) == -1) {
        perror("Erro no bind");
        exit(EXIT_FAILURE); 
    }

    // Prepara estrutura para ativar o modo promíscuo (captura todos os pacotes que passam pela interface)
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;                    // Índice da interface
    mr.mr_type = PACKET_MR_PROMISC;             // Ativa modo promíscuo

    // Solicita ao kernel que coloque a interface em modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        perror("Erro ao ativar modo promíscuo");
        exit(EXIT_FAILURE);
    }

    return soquete;             // Retorna o descritor do socket criado com sucesso
}

// Retorna o timestamp atual em milissegundos desde a época (Unix epoch)
long long timestamp() {
    struct timeval tp;                        // Estrutura para armazenar o tempo atual (segundos e microssegundos)
    gettimeofday(&tp, NULL);                  // Preenche a estrutura com o tempo atual (em segundos e microssegundos)

    // Converte o tempo para milissegundos:
    // - tp.tv_sec * 1000: segundos para milissegundos
    // - tp.tv_usec / 1000: microssegundos para milissegundos
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

// Calcula o checksum simples usando XOR entre os campos de controle e os dados
unsigned char calcula_checksum(unsigned char campo1, unsigned char campo2, unsigned char *dados, short tam_dados) {
    unsigned char checksum = campo1 ^ campo2;               // Inicia o checksum como o XOR dos dois campos de controle

    for (int i = 0; i < tam_dados; i++) {                   // Aplica XOR sucessivamente com cada byte dos dados
        checksum ^= dados[i];
    }

    return checksum;                                        // Retorna o resultado final como checksum
}

// Aguarda uma mensagem do servidor contendo a posição inicial do jogador
void espera_servidor(Mapa *m, int soquete, char *interface, int timeoutMillis, Frame *f, unsigned char* buffer, int tamanho_buffer) {
    unsigned char *resposta;        

    while (1) {
        unsigned char ack_type = 0x01;              // Por padrão, envia NACK (0x01) caso a mensagem não seja válida

        int status = recebe_mensagem(soquete, timeoutMillis, f, buffer, tamanho_buffer);                // Aguarda o recebimento de uma mensagem no socket, com timeout especificado

        if (status > 0 && f->tamanho_dados >= 2) {              // Se a mensagem foi recebida com sucesso e tem ao menos dois bytes de dados
            if (f->tipo == 0x03 && f->sequencia == 0) {         // Verifica se é o tipo de mensagem esperada (0x03) e se é a primeira da sequência (seq = 0)

                // Extrai linha e coluna iniciais do avatar a partir dos dados recebidos
                m->linha = f->dados[0];
                m->coluna = f->dados[1];

                ack_type = 0x00; // Define tipo de resposta como ACK (confirmação)
                printf("✔️ Posição inicial recebida: linha %d, coluna %d\n", m->linha, m->coluna);
            }

            resposta = monta_mensagem(0, 0, ack_type, NULL);                // Monta a resposta (ACK ou NACK) com tamanho 0, sequência 0 e tipo definido acima
            envia_mensagem(soquete, resposta, 4, interface);                // Envia a resposta pela interface especificada
            free(resposta);                                                 // Libera memória alocada para a resposta

            if (ack_type == 0x00) break;                                    // Se a mensagem foi válida e um ACK foi enviado, sai do loop
        }

        timeoutMillis *= 2;
        // Caso contrário, o loop continua tentando receber uma mensagem válida
    }
}

