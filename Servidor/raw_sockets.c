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


