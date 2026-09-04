#include <iostream>
#include <windows.h>
#include <string>
#include <cstdlib>

// Indica se o programa deve continuar executando
bool running = true;
int n_eventos = 10;

int handle(int i){
    std::cout << "Recebido SINAL " << std::to_string(i) << std::endl;
    
    if(i == 0){
        std::cout << "Encerrando programa..." << std::endl;
        running = false;
    }
    return i;
}

bool eventos_criados(HANDLE sinal[], int size){
    for (size_t i = 0; i < size; i++){
        if(sinal[i] == NULL) return false;
    }
    return true;
}

int receber(std::string modo){
    // -------------------------------------------------
    // Verifica os argumentos
    // -------------------------------------------------

    if (modo != "busy" && modo != "blocking"){
        std::cerr << "Erro: o modo deve ser "
                  << "'busy' ou 'blocking'."
                  << std::endl;

        return 1;
    }


    // -------------------------------------------------
    // Obtém o PID do processo atual
    // -------------------------------------------------

    DWORD pid = GetCurrentProcessId();


    std::cout << "Programa receptor iniciado." << std::endl;
    std::cout << "PID: " << pid << std::endl;
    std::cout << "Modo: " << modo << std::endl;


    // -------------------------------------------------
    // Cria os nomes dos eventos e os eventos
    // -------------------------------------------------

    std::string nomeSinal[n_eventos];
    HANDLE sinal[n_eventos];

    for(int i = 0; i < n_eventos; i++){
        std::cout << i << "\t";
        nomeSinal[i] = "Sinal" + 
                       std::to_string(i) + "_" + 
                       std::to_string(pid);
        sinal[i] = CreateEventA(
            NULL,       // atributos
            FALSE,      // auto-reset
            FALSE,      // inicialmente não sinalizado
            nomeSinal[i].c_str()
        );
    }

    // -------------------------------------------------
    // Verifica se os eventos foram criados
    // -------------------------------------------------

    if (!eventos_criados(sinal, n_eventos)){
        std::cerr << "\nErro ao criar os eventos."
                  << std::endl;

        for(int i = 0; i < n_eventos; i++){
            if(sinal[i] != NULL) CloseHandle(sinal[i]);
        }

        return 1;
    }

    std::cout << "\nEventos criados." << std::endl;
    std::cout << "Aguardando sinais..." << std::endl;

    // =================================================
    // BUSY WAIT
    // =================================================

    if (modo == "busy"){
        int recebido;
        while (running){
            // Verifica SINAL
            for(int i = 0; i < n_eventos; i++){
                if (WaitForSingleObject(sinal[i], 0) == WAIT_OBJECT_0){
                    recebido = handle(i);
                }
            }
        }
    }


    // =================================================
    // BLOCKING WAIT
    // =================================================

    else{
        while (running){
            DWORD resultado = WaitForMultipleObjects(
                n_eventos,          // quantidade de eventos
                sinal,    // eventos
                FALSE,      // espera qualquer um
                INFINITE     // espera indefinidamente
            );

            bool aux = true;
            for (int i = 0; i < n_eventos; i++){
                if(resultado == WAIT_OBJECT_0 + i){
                    handle(i);
                    aux = false;
                    break;
                }
            }
            if(aux){
                std::cerr << "Erro ao esperar pelos eventos."
                          << std::endl;

                running = false;
            }
        }
    }


    // -------------------------------------------------
    // Libera os eventos
    // -------------------------------------------------

    for (int i = 0; i < n_eventos; i++){
        CloseHandle(sinal[i]);
    }

    std::cout << "Programa encerrado." << std::endl;

    return 0;
}

int enviar(int pid, int sinal){

    if (pid == 0)
    {
        std::cerr << "Erro: PID invalido."
                  << std::endl;

        return 1;
    }


    if (sinal < 0 || sinal > 9)
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

    std::string io;
    std::string modo;
    int pid = 0;
    int sinal = 1;

    std::cout << "Digite Enviar ou Receber: ";
    std::cin >> io;

    if(io == "Enviar"){
        std::cout << "Digite o PID do alvo: ";
        std::cin >> pid;

        while (sinal!=0){
            std::cout << "Digite o sinal (use 0 para fechar): ";
            std::cin >> sinal;

            enviar(pid, sinal);
        }
    }
    if(io == "Receber"){
        std::cout << "Digite o MODO de espera blocking ou busy: ";
        std::cin >> modo;

        receber(modo);
    }    
}
