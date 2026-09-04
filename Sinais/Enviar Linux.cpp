#include <iostream>
#include <csignal>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>

int main(int argc, char* argv[]) {

    // Verifica quantidade de argumentos
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <PID> <SINAL>\n";
        std::cerr << "Exemplo: " << argv[0] << " 12345 10\n";
        return 1;
    }

    // Converte os argumentos
    pid_t pid = static_cast<pid_t>(std::atoi(argv[1]));
    int sinal = std::atoi(argv[2]);

    // Verifica se o PID é válido
    if (pid <= 0) {
        std::cerr << "Erro: PID invalido.\n";
        return 1;
    }

    // Tenta enviar o sinal
    if (kill(pid, sinal) == -1) {

        if (errno == ESRCH) {
            std::cerr << "Erro: o processo com PID "
                      << pid << " nao existe.\n";
        } 
        else if (errno == EPERM) {
            std::cerr << "Erro: sem permissao para enviar o sinal.\n";
        } 
        else {
            std::cerr << "Erro ao enviar o sinal.\n";
        }

        return 1;
    }

    std::cout << "Sinal " << sinal
              << " enviado para o processo "
              << pid << ".\n";

    return 0;
}