#ifndef RAW_SOCKETS_H
#define RAW_SOCKETS_H

#include <netinet/if_ether.h>           // Define constantes e estruturas da camada Ethernet
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include "mapa_cliente.h"
#include "enviar.h"
#include "receber.h"

// Cria um socket raw para interceptar e enviar pacotes diretamente na camada de enlace (Ethernet)
int cria_raw_socket(char* nome_interface_rede);

// Retorna o timestamp atual em milissegundos desde a época (Unix epoch)
long long timestamp();

// Calcula o checksum simples usando XOR entre os campos de tamanho, sequencia, tipo e os dados
unsigned char calcula_checksum(unsigned char campo1, unsigned char campo2, unsigned char *dados, short tam_dados);

// Aguarda uma mensagem do servidor contendo a posição inicial do jogador
void espera_servidor(Mapa *m, int soquete, char *interface, int timeoutMillis, Frame *f, unsigned char* buffer, int tamanho_buffer);

#endif
