#ifndef FRAME_H
#define FRAME_H

typedef struct {                        // Estrutura que representa um quadro de comunicação personalizado
    unsigned char marcador_inicio;      // Delimitador de início do quadro (sempre 0x7E)
    unsigned char campo1;               // Codifica o tamanho dos dados (7 bits) e MSB da sequência (1 bit)
    unsigned char campo2;               // Codifica os 4 LSB da sequência e o tipo da mensagem (4 bits)
    unsigned char checksum;             // Verificador de integridade (XOR dos campos e dados)
    unsigned char dados[127];           // Dados úteis (payload)
    short tamanho_dados;                // Tamanho dos dados
    unsigned char sequencia;            // Número de sequência completo (5 bits, 0–31)
    unsigned char tipo;                 // Tipo da mensagem: 0x00 = ACK, 0x01 = NACK, 0x02 = ACK+OK, etc.
} Frame;

#endif