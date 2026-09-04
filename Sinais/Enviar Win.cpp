#include <iostream>
#include <windows.h>
#include <string>
#include <cstdlib>


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Uso: "
                  << argv[0]
                  << " <PID> <SINAL>"
                  << std::endl;

        return 1;
    }


    DWORD pid = static_cast<DWORD>(std::atoi(argv[1]));
    int sinal = std::atoi(argv[2]);


    if (pid == 0)
    {
        std::cerr << "Erro: PID invalido."
                  << std::endl;

        return 1;
    }


    if (sinal < 1 || sinal > 3)
    {
        std::cerr << "Erro: o sinal deve ser 1, 2 ou 3."
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