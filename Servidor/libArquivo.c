#include "libArquivo.h"

// Detecta o tipo básico de um arquivo com base em seus primeiros bytes (assinatura mágica ou conteúdo)
const char* detectar_tipo_basico(const char* caminho) {
    unsigned char buffer[12];               // Buffer para ler os primeiros 12 bytes do arquivo
    FILE* fp = fopen(caminho, "rb");        // Abre o arquivo em modo binário para leitura
    if (!fp) {
        perror("Erro ao abrir arquivo"); 
        return "erro";
    }

    fread(buffer, 1, sizeof(buffer), fp);  // Lê até 12 bytes do início do arquivo
    fclose(fp);                             // Fecha o arquivo após leitura

    // Verifica se é um arquivo JPEG: assinatura começa com 0xFF 0xD8 0xFF
    if (buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF) {
        return "jpg";
    }

    // Verifica se é um arquivo MP4: procura a string "ftyp" nos primeiros 12 bytes
    for (int i = 0; i < 12 - 3; i++) {
        if (buffer[i] == 'f' && buffer[i+1] == 't' && buffer[i+2] == 'y' && buffer[i+3] == 'p') {
            return "mp4";
        }
    }

    // Verifica se é um arquivo de texto: apenas caracteres ASCII imprimíveis ou de controle
    int eh_texto = 1;
    for (int i = 0; i < sizeof(buffer); i++) {
        if ((buffer[i] < 32 || buffer[i] > 126) &&
            buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t') {
            eh_texto = 0;           // Encontrou caractere binário: não é texto
            break;
        }
    }

    if (eh_texto) return "texto";           // Retorna "texto" se todos os caracteres forem válidos

    return "desconhecido";                  // Caso não corresponda a nenhum tipo conhecido
}

// Procura um arquivo de tesouro na pasta "objetos" cujo nome contenha o índice fornecido
int achar_arquivo_tesouro(int indice, char* nome_saida) {
    DIR *dir;
    struct dirent *ent;
    char pasta[] = "./objetos";             // Caminho para a pasta onde estão os arquivos de tesouro
    char indice_str[8];                     // Buffer para armazenar o índice como string
    sprintf(indice_str, "%d", indice);      // Converte o índice inteiro para string

    // Tenta abrir a pasta
    if ((dir = opendir(pasta)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {              // Itera sobre cada entrada da pasta
            if (strstr(ent->d_name, indice_str)) {          // Verifica se o nome do arquivo contém o índice (ex: "tesouro_3.jpg")
                strncpy(nome_saida, ent->d_name, 63);       // Copia o nome do arquivo encontrado para nome_saida (com limite de 63 caracteres)
                nome_saida[63] = '\0';                      // Garante terminação da string

                // Monta o caminho completo do arquivo
                char caminho_completo[512];
                snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", pasta, ent->d_name);

                const char* tipo = detectar_tipo_basico(caminho_completo);          // Detecta o tipo básico do arquivo

                // Retorna o código correspondente ao tipo do arquivo
                if (strcmp(tipo, "texto") == 0) return 6;
                if (strcmp(tipo, "mp4") == 0) return 7;
                if (strcmp(tipo, "jpg") == 0) return 8;

                return 15;       // Tipo desconhecido
            }
        }
        closedir(dir);          // Fecha o diretório após a leitura
    } 
    
    else {
        // Caso não consiga abrir a pasta
        perror("Erro ao abrir pasta objetos");
        return 15;
    }

    return 0;  // Nenhum arquivo correspondente encontrado
}

// Retorna o tamanho, em bytes, de um arquivo localizado na pasta "objetos"
int tamanho_arquivo(char* nome_arquivo) {
    
    char caminho[72];          // Buffer para armazenar o caminho completo até o arquivo

    // Copia "objetos/" para o buffer e concatena o nome do arquivo
    strncpy(caminho, "objetos/", sizeof(caminho) - 1);          // Garante que não ultrapasse o tamanho
    caminho[sizeof(caminho) - 1] = '\0';                        // Garante que a string está terminada corretamente
    strncat(caminho, nome_arquivo, sizeof(caminho) - strlen(caminho) - 1);          // Adiciona nome do arquivo

    struct stat st;         // Estrutura para armazenar informações sobre o arquivo

    if (stat(caminho, &st) != 0) {          // Obtém informações do arquivo especificado
        perror("Erro ao obter informações do arquivo");  
        return 15;
    }

    if (!S_ISREG(st.st_mode)) {             // Verifica se o caminho aponta para um arquivo regular
        fprintf(stderr, "O caminho não é um arquivo regular\n");
        return 15;
    }
    return (int) st.st_size;                // Retorna o tamanho do arquivo
}

// Envia arquivo do tesouro encontrado
int envia_arquivo(int soquete, char* iface, char* caminho, char unsigned *seq, int tesouro){
    FILE* arq = fopen(caminho, "rb");           // Abre o arquivo em modo binário
    if (!arq) {
        perror("Erro ao abrir arquivo");
        return 15;
    }

    unsigned char buffer[127];                  // Pacote de dados (máx 127 bytes)
    size_t lidos;

    while ((lidos = fread(buffer, 1, 127, arq)) > 0) {          // Envia o conteúdo do arquivo em blocos de até 127 bytes
        if(!envia_com_ack(soquete, iface, buffer, lidos, 5, seq)){
            printf("❌ Falha ao enviar arquivo do tesouro %d\n", tesouro);  
            return 15;
        }
    }

    fclose(arq);

    *seq = (*seq + 1) % 32;

    if(!envia_com_ack(soquete, iface, NULL, 0, 9, seq)){        // Envia mensagem especial de fim de arquivo
        printf("❌ Falha ao enviar fim de arquivo do tesouro %d\n", tesouro);  
        return 15;
    }

    else printf("📦 Fim do arquivo enviado\n");
    
    return 0;
}