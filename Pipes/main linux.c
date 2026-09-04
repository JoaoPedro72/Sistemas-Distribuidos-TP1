#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define TAM 20

int eh_primo(int numero) {
    if (numero < 2)
        return 0;

    if (numero == 2)
        return 1;

    if (numero % 2 == 0)
        return 0;

    for (int i = 3; i * i <= numero; i += 2) {
        if (numero % i == 0)
            return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <quantidade>\n", argv[0]);
        return 1;
    }

    int quantidade = atoi(argv[1]);

    if (quantidade <= 0) {
        fprintf(stderr, "A quantidade deve ser maior que zero.\n");
        return 1;
    }

    int pipefd[2];

    /*
     * pipefd[0] -> leitura
     * pipefd[1] -> escrita
     */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    /* =========================
       PROCESSO FILHO - CONSUMIDOR
       ========================= */
    if (pid == 0) {

        close(pipefd[1]); // filho não escreve

        char buffer[TAM];

        while (1) {

            ssize_t bytes = read(pipefd[0], buffer, TAM);

            if (bytes == -1) {
                perror("read");
                close(pipefd[0]);
                exit(1);
            }

            if (bytes == 0) {
                break;
            }

            /*
             * Como o produtor sempre envia exatamente
             * TAM bytes, podemos adicionar o '\0'
             * para transformar o conteúdo em string.
             */
            buffer[TAM - 1] = '\0';

            int numero = atoi(buffer);

            if (numero == 0) {
                break;
            }

            if (eh_primo(numero))
                printf("Consumidor: %d -> primo\n", numero);
            else
                printf("Consumidor: %d -> nao primo\n", numero);
        }

        close(pipefd[0]);

        return 0;
    }

    /* =========================
       PROCESSO PAI - PRODUTOR
       ========================= */

    close(pipefd[0]); // pai não lê

    srand(time(NULL));

    int numero = 1;

    for (int i = 0; i < quantidade; i++) {

        char buffer[TAM];

        /*
         * Gera delta entre 1 e 100.
         *
         * O primeiro número é 1.
         * Nos próximos:
         *
         * Ni = Ni-1 + delta
         */
        if (i > 0) {
            int delta = (rand() % 100) + 1;
            numero += delta;
        }

        /*
         * Converte o número para uma string
         * de exatamente TAM bytes.
         */
        snprintf(buffer, TAM, "%-19d", numero);

        /*
         * Envia exatamente TAM bytes.
         */
        if (write(pipefd[1], buffer, TAM) != TAM) {
            perror("write");
            close(pipefd[1]);
            return 1;
        }

        printf("Produtor: %d\n", numero);
    }

    /*
     * Envia o número 0 para avisar o consumidor
     * que não existem mais números.
     */
    char buffer[TAM];
    snprintf(buffer, TAM, "%-19d", 0);

    if (write(pipefd[1], buffer, TAM) != TAM) {
        perror("write");
    }

    close(pipefd[1]);

    /*
     * Espera o consumidor terminar.
     */
    wait(NULL);

    return 0;
}