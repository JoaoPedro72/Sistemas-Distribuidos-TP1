#include <iostream>
#include <csignal>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void handleSIGUSR1(int signal) {
    const char mensagem[] = "Recebido SIGUSR1\n";
    write(STDOUT_FILENO, mensagem, sizeof(mensagem) - 1);
}

void handleSIGUSR2(int signal) {
    const char mensagem[] = "Recebido SIGUSR2\n";
    write(STDOUT_FILENO, mensagem, sizeof(mensagem) - 1);
}

void handleSIGINT(int signal) {
    const char mensagem[] = "Recebido SIGINT. Encerrando...\n";
    write(STDOUT_FILENO, mensagem, sizeof(mensagem) - 1);

    running = 0;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <busy|blocking>\n";
        return 1;
    }

    std::string modo = argv[1];

    if (modo != "busy" && modo != "blocking") {
        std::cerr << "Erro: use 'busy' ou 'blocking'.\n";
        return 1;
    }

    // Configura os handlers
    signal(SIGUSR1, handleSIGUSR1);
    signal(SIGUSR2, handleSIGUSR2);
    signal(SIGINT, handleSIGINT);

    std::cout << "Programa iniciado.\n";
    std::cout << "PID: " << getpid() << "\n";
    std::cout << "Modo: " << modo << "\n";
    std::cout << "Aguardando sinais...\n";

    if (modo == "busy") {

        // Busy wait
        while (running) {
            // O processo fica consumindo CPU
        }

    } else {

        // Blocking wait
        while (running) {
            pause();
        }
    }

    std::cout << "Programa encerrado.\n";

    return 0;
}