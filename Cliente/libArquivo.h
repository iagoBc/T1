#ifndef LIBARQUIVO_H
#define LIBARQUIVO_H

#define MARGEM_TOLERANCIA 1024 

#include <unistd.h>   // para fork(), execlp(), close()
#include <sys/types.h> // para pid_t
#include <sys/statvfs.h>

#include "frame.h"
#include "raw_sockets.h"  

extern FILE* arquivo;
extern int tamanho_arq;
extern char* caminho;

// Retorna o espaço livre disponível em disco (em bytes) para o caminho especificado
unsigned long long espaco_disco_disponivel(const char* caminho);

// Verifica se há espaço suficiente em disco para salvar um arquivo de tamanho dado
int pode_salvar_arquivo(const char* caminho, int tamanho_arquivo);

// Função para receber e salvar um arquivo enviado via socket RAW usando o protocolo definido
int baixa_arquivo(int soquete, char* iface, Frame *f, char unsigned *seq);


#endif