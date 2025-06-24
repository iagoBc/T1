#include "servidor.h"

void envia_erro(int soquete, char *interface, unsigned char *sequencia){
    unsigned char* resposta;

    resposta = monta_mensagem(0, *sequencia, 0x0F, NULL);
    envia_mensagem(soquete, resposta, 4, interface);
    free(resposta);
}

int todos_tesouros_encontrados(Mapa *m){
    for(int i = 0; i < 8; i++) {
        int l = m->tesouro[i].linhaT;
        int c = m->tesouro[i].colunaT;
        // Se a posição do tesouro não foi visitada, retorna 0
        if (m->visitas[l][c] == 0)
            return 0;
    }
    return 1;
}

// Inicializa servidor com 8 tesouros em posiçoes diferentes e posição inicial aleatoria
void inicializa_servidor(Mapa *m){
    int ocupadas[8][8] = {0};
    memset(m->visitas, 0, sizeof(m->visitas));
    int count = 0;
    srand(time(NULL));

    //Define posiçoes dos tesouros
    while (count < 8){
        int l = rand() % 8;
        int c = rand() % 8;
        if (!ocupadas[l][c]){
            ocupadas[l][c] = 1;
            m->tesouro[count].linhaT = l;
            m->tesouro[count].colunaT = c;
            count++;
        }
    }

    //Define posiçao inicial diferente dos tesouros
    while (1){
        int l = rand() % 8;
        int c = rand() % 8;
        if (!ocupadas[l][c]){
            m->linha = l;
            m->coluna = c;
            m->visitas[l][c] = 1;
            break;
        }
    }

}

// Envia para o cliente posição inicial do personagem
void envia_posi_inicio(Mapa *m, int soquete, char *interface, unsigned char* seq){
    unsigned char tipo = 0x03;          // Tipo 3 indica envio de posição inicial
    unsigned char dados[2];
    dados[0] = m->linha;                // Linha atual do personagem no mapa
    dados[1] = m->coluna;               // Coluna atual do personagem no mapa

    int retorno = envia_com_ack(soquete, interface, dados, 2, tipo, seq);           // Envia os dados com garantia de entrega e espera por ACK

    
    if(retorno == 1)                    // Se receber pelo menos ACK simples (tipo 0), considera que o cliente recebeu
        printf("Servidor inicializado e coordenadas de inicio estabelecidas (%d)(%d). Aguardando comandos!", m->linha, m->coluna);
}

// Retorna o tipo de ack especifico para o movimento
unsigned char trata_comando(Frame *f, Mapa* mapa){
    if (f->tipo == 0x0A && mapa->coluna < 7) {
        return 0x02;            // ACK + OK
    } 
    
    else if (f->tipo == 0x0B && mapa->linha < 7) {
        return 0x02;            // ACK + OK
    } 
    
    else if (f->tipo == 0x0C && mapa->linha > 0) {
        return 0x02;            // ACK + OK
    } 
    
    else if (f->tipo == 0x0D && mapa->coluna > 0) {
        return 0x02;            // ACK + OK
    } 
    
    else {
        return 0x00;            // ACK (movimento inválido, mas recebido)
    }
}

// Retorna 1 se na posição atual do avatar tem um tesouro
int encontrou_tesouro(Mapa *m){
    if(m->visitas[m->linha][m->coluna] == 1) return 0;          // Tesouro já foi coletado anteriormente

    // Percorre a lista de tesouros cadastrados
    for (int i = 0; i < 8; i++) {
        if (m->tesouro[i].linhaT == m->linha &&
            m->tesouro[i].colunaT == m->coluna) {              // Verifica se há um tesouro na posição atual do avatar
            return 1;                                          // Tesouro encontrado na posição atual
        }
    }

    return 0;                                                  // Nenhum tesouro encontrado nesta posição
}

// Trata o evento de encontro com um tesouro: envia nome, tipo, tamanho e conteúdo do arquivo associado
void trata_encontro(Mapa *m, int soquete, char *interface, unsigned char *sequencia){
    int tesouro;

    for (int i = 0; i < 8; i++) {       // Identifica qual tesouro foi encontrado com base na posição atual
        if (m->tesouro[i].linhaT == m->linha &&
            m->tesouro[i].colunaT == m->coluna) {
            tesouro = i + 1;            // Os arquivos têm índice 1 a 8
        }
    }

    char nome_arquivo[64];
    int tipo = achar_arquivo_tesouro(tesouro, nome_arquivo);        // Busca o nome do arquivo associado ao tesouro encontrado

    printf("🔍 Enviando arquivo do tesouro %d: nome = \"%s\", tipo = %d\n", tesouro, nome_arquivo, tipo);

    if (tipo >= 6 && tipo <= 8){            // Se o tipo for conhecido (texto, mp4 ou jpg)
        unsigned char dados[64];
        // Copia o nome do arquivo para o buffer de dados
        strncpy((char*)dados, nome_arquivo, 63);
        dados[63] = '\0';

        // Envia o nome do arquivo ao cliente
        if (!envia_com_ack(soquete, interface, dados, strlen((char*)dados), tipo, sequencia)) {
            printf("❌ Falha ao enviar nome do arquivo do tesouro %d\n", tesouro);
            envia_erro(soquete, interface, sequencia);
        } 
        
        else {
            printf("📦 Nome do arquivo enviado: %s (tipo %d)\n", nome_arquivo, tipo);

            // Descobre o tamanho do arquivo
            int tamanho = tamanho_arquivo(nome_arquivo);
            unsigned char tam_arq[4];
            tam_arq[0] = (tamanho >> 24) & 0xFF;
            tam_arq[1] = (tamanho >> 16) & 0xFF;
            tam_arq[2] = (tamanho >> 8) & 0xFF;
            tam_arq[3] = tamanho & 0xFF;

            *sequencia = (*sequencia + 1) % 32;             // Avança sequência

            if(!envia_com_ack(soquete, interface, tam_arq, 4, 4, sequencia)){           // Envia o tamanho do arquivo
                printf("❌ Falha ao enviar tamanho do arquivo do tesouro %d\n", tesouro);
                envia_erro(soquete, interface, sequencia);
            }
            else{
                printf("📦 Tamanho do arquivo enviado: %d bytes\n", tamanho);

                // Monta o caminho completo para o arquivo dentro da pasta "objetos/"
                char caminho[63 + 9];  // "objetos/" + nome (máx 63)
                strncpy(caminho, "objetos/", sizeof(caminho) - 1);
                caminho[sizeof(caminho) - 1] = '\0';
                strncat(caminho, nome_arquivo, sizeof(caminho) - strlen(caminho) - 1);

                if(envia_arquivo(soquete, interface, caminho, sequencia, tesouro) == 15)
                    envia_erro(soquete, interface, sequencia);
            }
        }
    }
    else if (tipo == 15){
        printf("Erro no tipo do arquivo. Não é txt, mp4 ou jpg");
        envia_erro(soquete, interface, sequencia);
    }
}
