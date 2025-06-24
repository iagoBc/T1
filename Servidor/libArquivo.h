#ifndef __LIBARQUIVO__
#define __LIBARQUIVO_

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "raw_sockets.h"

// Detecta o tipo básico de um arquivo com base em seus primeiros bytes (assinatura mágica ou conteúdo)
const char* detectar_tipo_basico(const char* caminho);

// Procura um arquivo de tesouro na pasta "objetos" cujo nome contenha o índice fornecido
int achar_arquivo_tesouro(int indice, char* nome_saida);

// Retorna o tamanho, em bytes, de um arquivo localizado na pasta "objetos"
int tamanho_arquivo(char* nome_arquivo);

// Envia arquivo do tesouro encontrado
int envia_arquivo(int soquete, char* iface, char* caminho, char unsigned *seq, int tesouro);
#endif