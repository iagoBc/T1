#include "receber.h"

int parse_frame(unsigned char* buffer, int tamanho_buffer, Frame* f) {                                                  // Converte um buffer de bytes em um Frame, se a estrutura for válida
    for (int i = 0; i < tamanho_buffer; i++) {
        // Verifica o marcador de início do frame
        if (buffer[i] == 0x7E && i + 4 <= tamanho_buffer) {
            f->marcador_inicio = 0x7E;
            f->campo1 = buffer[i + 1];
            f->campo2 = buffer[i + 2];
            f->checksum = buffer[i + 3];

            f->tamanho_dados = f->campo1 >> 1;                                                                          // Extrai o tamanho dos dados (7 bits mais significativos de campo1)

            unsigned char seq_msb = f->campo1 & 0x01;                                                                   // Extrai a sequência (1 bit de campo1 e 4 bits de campo2)
            unsigned char seq_lsb = (f->campo2 & 0xF0) >> 4;
            f->sequencia = (seq_msb << 4) | seq_lsb;

            f->tipo = f->campo2 & 0x0F;                                                                                 // Extrai o tipo de mensagem (últimos 4 bits de campo2)

            if (i + 4 + f->tamanho_dados > tamanho_buffer) return -1;                                                   // Verifica se o buffer contém todos os dados declarados

            memcpy(f->dados, &buffer[i + 4], f->tamanho_dados);

            unsigned char cs = calcula_checksum(f->campo1, f->campo2, f->dados, f->tamanho_dados);                      // Verifica o checksum
            if (cs == f->checksum) {
                return f->tamanho_dados + 4;                                                                            // Frame válido
            }
        }
    }
    return -1;                                                                                                          // Nenhum frame válido encontrado
}

int recebe_mensagem(int soquete, int timeoutMillis, Frame* f, unsigned char* buffer, int tamanho_buffer) {              // Recebe uma mensagem do socket dentro de um timeout especificado
    long long inicio = timestamp();

    struct timeval timeout = { .tv_sec = timeoutMillis / 1000, .tv_usec = (timeoutMillis % 1000) * 1000 };              // Define timeout para recepção
    setsockopt(soquete, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    int lidos;
    while (timestamp() - inicio <= timeoutMillis) {
        lidos = recv(soquete, buffer, tamanho_buffer, 0);                                                               // Lê do socket
        if (lidos > 0) {
            if (parse_frame(buffer, lidos, f) > 0) {                                                                    // Tenta extrair um frame válido
                return lidos;                                                                                               // Sucesso
            }
        }
    }
    return -1;                                                                                                          // Timeout ou erro
}