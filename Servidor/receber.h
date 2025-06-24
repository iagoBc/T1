#ifndef RECEBER_H
#define RECEBER_H

#include "frame.h"
#include "raw_sockets.h"

// Analisa o buffer recebido procurando por um quadro válido e preenche a estrutura Frame
int parse_frame(unsigned char* buffer, int tamanho_buffer, Frame* f);

// Recebe uma mensagem do socket, tentando extrair um quadro válido dentro do tempo limite
int recebe_mensagem(int soquete, int timeoutMillis, Frame* f, unsigned char* buffer, int tamanho_buffer);

#endif