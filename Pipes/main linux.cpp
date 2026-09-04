#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

const int TAMANHO_MENSAGEM = 20;


// Verifica se um número é primo
bool ehPrimo(int numero) {
    int metade = numero/2;

    if (numero < 2) {
        return false;
    }

    for (int i = 2; i <= metade; i++) {
        if (numero % i == 0) {
            return false;
        }
    }

    return true;
}


// Processo produtor
void produtor(int fd[2], int quantidade) {

    // O produtor não utiliza a ponta de leitura
    close(fd[0]);

    int numero = 1;

    for (int i = 0; i < quantidade; i++) {

        // Gera delta entre 1 e 100
        int delta = rand() % 100 + 1;

        // N_i = N_(i-1) + delta
        if (i > 0) {
            numero += delta;
        }

        char mensagem[TAMANHO_MENSAGEM];

        // Inicializa a mensagem com zeros
        memset(mensagem, 0, TAMANHO_MENSAGEM);

        // Converte o número para string
        snprintf(
            mensagem,
            TAMANHO_MENSAGEM,
            "%d",
            numero
        );

        // Escreve exatamente 20 bytes no pipe
        write(
            fd[1],
            mensagem,
            TAMANHO_MENSAGEM
        );

        std::cout << "[Produtor] Enviado: "
                  << numero
                  << std::endl;
    }

    // Envia o número 0 para indicar fim
    char mensagem[TAMANHO_MENSAGEM];

    memset(mensagem, 0, TAMANHO_MENSAGEM);

    snprintf(
        mensagem,
        TAMANHO_MENSAGEM,
        "%d",
        0
    );

    write(
        fd[1],
        mensagem,
        TAMANHO_MENSAGEM
    );

    std::cout << "[Produtor] Enviado: 0 (fim)"
              << std::endl;

    // Fecha a ponta de escrita
    close(fd[1]);
}


// Processo consumidor
void consumidor(int fd[2]) {

    // O consumidor não utiliza a ponta de escrita
    close(fd[1]);

    while (true) {

        char mensagem[TAMANHO_MENSAGEM];

        // Limpa o buffer
        memset(mensagem, 0, TAMANHO_MENSAGEM);

        // Lê exatamente 20 bytes
        ssize_t bytes_lidos = read(
            fd[0],
            mensagem,
            TAMANHO_MENSAGEM
        );

        // Verifica se houve erro ou fim do pipe
        if (bytes_lidos <= 0) {
            break;
        }

        // Converte a string para inteiro
        int numero = atoi(mensagem);

        // 0 indica que o produtor terminou
        if (numero == 0) {

            std::cout
                << "[Consumidor] Recebido 0. "
                << "Encerrando."
                << std::endl;

            break;
        }

        // Verifica se é primo
        if (ehPrimo(numero)) {

            std::cout
                << "[Consumidor] "
                << numero
                << " -> PRIMO"
                << std::endl;

        } else {

            std::cout
                << "[Consumidor] "
                << numero
                << " -> NAO PRIMO"
                << std::endl;
        }
    }

    // Fecha a ponta de leitura
    close(fd[0]);
}


int main(int argc, char *argv[]) {

    // Verifica o parâmetro
    if (argc != 2) {

        std::cerr
            << "Uso: "
            << argv[0]
            << " <quantidade>"
            << std::endl;

        return 1;
    }

    int quantidade = atoi(argv[1]);

    if (quantidade <= 0) {

        std::cerr
            << "Erro: a quantidade deve ser maior que zero."
            << std::endl;

        return 1;
    }


    // Cria o pipe
    int fd[2];

    if (pipe(fd) == -1) {

        std::cerr
            << "Erro ao criar o pipe."
            << std::endl;

        return 1;
    }


    // Duplica o processo
    pid_t pid = fork();

    if (pid == -1) {

        std::cerr
            << "Erro ao executar fork()."
            << std::endl;

        close(fd[0]);
        close(fd[1]);

        return 1;
    }


    if (pid == 0) {

        // FILHO = CONSUMIDOR
        consumidor(fd);

        return 0;

    } else {

        // PAI = PRODUTOR
        produtor(fd, quantidade);

        // Espera o filho terminar
        waitpid(pid, nullptr, 0);

        std::cout
            << "[Produtor] Consumidor terminou."
            << std::endl;
    }

    return 0;
}