#include <iostream>
#include <windows.h>
#include <string>

// Indica se o programa deve continuar executando
bool running = true;


// =====================================================
// HANDLERS
// =====================================================

void handleSinal1()
{
    std::cout << "Recebido SINAL 1" << std::endl;
}


void handleSinal2()
{
    std::cout << "Recebido SINAL 2" << std::endl;
}


void handleSinal3()
{
    std::cout << "Recebido SINAL 3" << std::endl;
    std::cout << "Encerrando programa..." << std::endl;

    running = false;
}


// =====================================================
// MAIN
// =====================================================

int main(int argc, char* argv[])
{
    // -------------------------------------------------
    // Verifica os argumentos
    // -------------------------------------------------

    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <busy|blocking>"
                  << std::endl;

        return 1;
    }


    std::string modo = argv[1];


    if (modo != "busy" && modo != "blocking")
    {
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
    // Cria os nomes dos eventos
    // -------------------------------------------------

    std::string nomeSinal1 =
        "Sinal1_" + std::to_string(pid);

    std::string nomeSinal2 =
        "Sinal2_" + std::to_string(pid);

    std::string nomeSinal3 =
        "Sinal3_" + std::to_string(pid);


    // -------------------------------------------------
    // Cria os eventos
    // -------------------------------------------------

    HANDLE sinal1 = CreateEventA(
        NULL,       // atributos
        FALSE,      // auto-reset
        FALSE,      // inicialmente não sinalizado
        nomeSinal1.c_str()
    );


    HANDLE sinal2 = CreateEventA(
        NULL,
        FALSE,
        FALSE,
        nomeSinal2.c_str()
    );


    HANDLE sinal3 = CreateEventA(
        NULL,
        FALSE,
        FALSE,
        nomeSinal3.c_str()
    );


    // -------------------------------------------------
    // Verifica se os eventos foram criados
    // -------------------------------------------------

    if (sinal1 == NULL ||
        sinal2 == NULL ||
        sinal3 == NULL)
    {
        std::cerr << "Erro ao criar os eventos."
                  << std::endl;

        if (sinal1 != NULL)
            CloseHandle(sinal1);

        if (sinal2 != NULL)
            CloseHandle(sinal2);

        if (sinal3 != NULL)
            CloseHandle(sinal3);

        return 1;
    }


    std::cout << "Eventos criados." << std::endl;
    std::cout << "Aguardando sinais..." << std::endl;


    // =================================================
    // BUSY WAIT
    // =================================================

    if (modo == "busy")
    {
        while (running)
        {
            // Verifica SINAL 1
            if (WaitForSingleObject(sinal1, 0)
                == WAIT_OBJECT_0)
            {
                handleSinal1();
            }


            // Verifica SINAL 2
            if (WaitForSingleObject(sinal2, 0)
                == WAIT_OBJECT_0)
            {
                handleSinal2();
            }


            // Verifica SINAL 3
            if (WaitForSingleObject(sinal3, 0)
                == WAIT_OBJECT_0)
            {
                handleSinal3();
            }
        }
    }


    // =================================================
    // BLOCKING WAIT
    // =================================================

    else
    {
        HANDLE eventos[] =
        {
            sinal1,
            sinal2,
            sinal3
        };


        while (running)
        {
            DWORD resultado = WaitForMultipleObjects(
                3,          // quantidade de eventos
                eventos,    // eventos
                FALSE,      // espera qualquer um
                INFINITE     // espera indefinidamente
            );


            if (resultado == WAIT_OBJECT_0)
            {
                // Sinal 1
                handleSinal1();
            }
            else if (resultado == WAIT_OBJECT_0 + 1)
            {
                // Sinal 2
                handleSinal2();
            }
            else if (resultado == WAIT_OBJECT_0 + 2)
            {
                // Sinal 3
                handleSinal3();
            }
            else
            {
                std::cerr << "Erro ao esperar pelos eventos."
                          << std::endl;

                running = false;
            }
        }
    }


    // -------------------------------------------------
    // Libera os eventos
    // -------------------------------------------------

    CloseHandle(sinal1);
    CloseHandle(sinal2);
    CloseHandle(sinal3);


    std::cout << "Programa encerrado." << std::endl;


    return 0;
}