#ifndef ENVIAR_H
#define ENVIAR_H

#include <sys/ioctl.h>

#include "frame.h"
#include "raw_sockets.h"
#include "libArquivo.h"


extern FILE* arquivo;
extern int tamanho_arq;
extern char* caminho;

// Monta um frame para envio com os campos de controle (cabeçalho), dados e checksum
unsigned char* monta_mensagem(short tamanho, unsigned char sequencia, unsigned char tipo, unsigned char *dados);

// Envia uma mensagem via raw socket para a interface de rede especificada
void envia_mensagem(int soquete, unsigned char* dados, int tamanho_dados, char* nome_interface_rede);

// Envia uma mensagem via socket RAW e espera por um ACK correspondente
int envia_com_ack(int soquete, char* iface, unsigned char* dados, int tamanho, unsigned char tipo, unsigned char *seq);

#endif