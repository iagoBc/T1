#include "libArquivo.h"

// Retorna o espaço livre disponível em disco (em bytes) para o caminho especificado
unsigned long long espaco_disco_disponivel(const char* caminho){
    struct statvfs st;  // Estrutura para armazenar informações do sistema de arquivos

    // Obtém informações do sistema de arquivos referente ao caminho informado
    if (statvfs(caminho, &st) != 0) {
        perror("Erro ao obter informações do sistema de arquivos");
        return 15;   // Retorna indicando erro
    }

    unsigned long long espaco_livre = st.f_bsize * st.f_bavail;         // Calcula espaço livre disponível multiplicando o tamanho do bloco pelo número de blocos disponíveis para o usuário

    return espaco_livre;            // Retorna o espaço livre em bytes no sistema de arquivos 
}

// Verifica se há espaço suficiente em disco para salvar um arquivo de tamanho dado
int pode_salvar_arquivo(const char* caminho, int tamanho_arquivo) {
    unsigned long long livre = espaco_disco_disponivel(caminho);            // Consulta espaço livre disponível

    if (livre == 15) return 15;             // Retorna 15 se houve erro ao obter espaço disponível

    if (livre >= tamanho_arquivo + MARGEM_TOLERANCIA) {             // Compara o espaço livre com o tamanho do arquivo + uma margem de segurança definida
        return 1;               // Retorna 1 indicando que há espaço suficiente para salvar o arquivo
    } 
    
    else {
        fprintf(stderr, "❌ Espaço insuficiente para salvar o arquivo.\n");
        return 15;              // Retorna 15 indicando espaço insuficiente
    }
}

// Função para receber e salvar um arquivo enviado via socket RAW usando o protocolo definido
int baixa_arquivo(int soquete, char* iface, Frame *f, char unsigned *seq){
    int timeout = 500;                            // Timeout inicial para recepção em milissegundos
    unsigned char buffer[2048];                   // Buffer para armazenar dados recebidos

    // Envia ACK para o nome do arquivo recebido (frame sem dados, só com sequência e tipo 0)
    unsigned char* msg = monta_mensagem(0, f->sequencia, 0, NULL);
    envia_mensagem(soquete, msg, 4, iface);

    // Espera o próximo frame que deve conter o tamanho do arquivo (4 bytes)
    if(recebe_mensagem(soquete, timeout, f, buffer, sizeof(buffer)) > 0){
        tamanho_arq = (f->dados[0] << 24) | (f->dados[1] << 16) | (f->dados[2] << 8) | f->dados[3];         // Extrai o tamanho do arquivo a partir dos 4 primeiros bytes dos dados do frame

        if(pode_salvar_arquivo("objetos", tamanho_arq) == 15){              // Verifica se há espaço suficiente para salvar o arquivo no diretório "objetos"
            return 15;  // Retorna erro 15 para falta de espaço
        }

        // Abre o arquivo para escrita binária no caminho global 'caminho'
        arquivo = fopen(caminho, "wb");
        if (arquivo == NULL) {
            perror("Erro ao criar o arquivo");
            return 15;          // Retorna erro se não conseguiu abrir o arquivo
        }

        // Envia ACK para confirmar recepção do tamanho do arquivo
        msg = monta_mensagem(0, f->sequencia, 0, NULL);
        envia_mensagem(soquete, msg, 4, iface);

        if(recebe_mensagem(soquete, timeout, f, buffer, sizeof(buffer)) > 0){               // Espera receber os dados do arquivo
            if (f->tipo == 5) {                 // Tipo 5 indica pacote de dados do arquivo
                if (arquivo == NULL) {
                    fprintf(stderr, "Erro: arquivo não aberto para escrita\n");
                    free(msg);
                    return 15;
                }
                
                do {
                    fwrite(f->dados, 1, f->tamanho_dados, arquivo);             // Escreve os dados recebidos no arquivo

                    // Libera e cria um novo frame para enviar ACK
                    free(msg);
                    msg = monta_mensagem(0, f->sequencia, 0, NULL);
                    envia_mensagem(soquete, msg, 4, iface);

                    recebe_mensagem(soquete, timeout, f, buffer, sizeof(buffer));           // Recebe o próximo frame com dados ou sinal de fim/erro

                    if (f->tipo == 15) {                // Se tipo 15, indica erro ou falta de espaço durante transferência
                        if (arquivo) fclose(arquivo);
                        arquivo = NULL;
                        return 15;
                    }
                } while (f->tipo == 5);                 // Continua recebendo enquanto for dados do arquivo

                if (f->tipo == 9) {             // Tipo 9 indica fim do arquivo
                    printf("\033[H\033[J"); 
                    fclose(arquivo);            // Fecha arquivo
                    printf("✔️ Recebido tesouro completo");

                    arquivo = NULL;

                    // Envia ACK final confirmando recepção completa
                    msg = monta_mensagem(0, f->sequencia, 0, NULL);
                    envia_mensagem(soquete, msg, 4, iface);
                    recebe_mensagem(soquete, timeout, f, buffer, sizeof(buffer));

                    *seq = f->sequencia;            // Atualiza sequência

                    pid_t pid = fork();             // Cria um processo filho para abrir o arquivo recebido automaticamente
                    if (pid == 0) {
                        execlp("xdg-open", "xdg-open", caminho, (char *)NULL);          // Processo filho executa xdg-open para abrir o arquivo no sistema padrão
                        
                        // Se execlp falhar, exibe erro e termina
                        perror("execlp");
                        exit(1);
                    }

                    return 0;
                }
            }
        }
    }
    
    return 15;          // Retorna erro
}

