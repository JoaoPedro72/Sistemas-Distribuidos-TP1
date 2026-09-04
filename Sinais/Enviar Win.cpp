#include <iostream>
#include <windows.h>
#include <string>
#include <cstdlib>

int n_eventos = 10;

int enviar(int pid, int sinal){

    if (pid == 0)
    {
        std::cerr << "Erro: PID invalido."
                  << std::endl;

        return 1;
    }


    if (sinal < 0 || sinal > n_eventos-1)
    {
        std::cerr << "Erro: o sinal deve ser entre 0 e 9."
                  << std::endl;

        return 1;
    }


    // Verifica se o processo existe
    HANDLE processo = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        pid
    );


    if (processo == NULL)
    {
        std::cerr << "Erro: processo com PID "
                  << pid
                  << " nao existe ou nao pode ser acessado."
                  << std::endl;

        return 1;
    }


    CloseHandle(processo);


    // Monta o nome do evento
    std::string nomeEvento =
        "Sinal" +
        std::to_string(sinal) +
        "_" +
        std::to_string(pid);


    // Abre o evento existente
    HANDLE evento = OpenEventA(
        EVENT_MODIFY_STATE,
        FALSE,
        nomeEvento.c_str()
    );


    if (evento == NULL)
    {
        std::cerr << "Erro: o processo existe, "
                  << "mas o evento nao foi encontrado."
                  << std::endl;

        return 1;
    }


    // Envia o sinal
    if (!SetEvent(evento))
    {
        std::cerr << "Erro ao enviar o sinal."
                  << std::endl;

        CloseHandle(evento);

        return 1;
    }


    std::cout << "Sinal "
              << sinal
              << " enviado para o processo "
              << pid
              << "."
              << std::endl;


    CloseHandle(evento);

    return 0;
}

int main(){
    int pid = 0;
    int sinal = 1;

    std::cout << "Digite o PID do alvo: ";
    std::cin >> pid;

    while (sinal!=0){
        std::cout << "Digite o sinal (use 0 para fechar): ";
        std::cin >> sinal;

        enviar(pid, sinal);
    }
}
