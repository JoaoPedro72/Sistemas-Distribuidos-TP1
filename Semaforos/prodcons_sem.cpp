// TP1 - Sistemas Distribuidos - CEFET-MG
// Parte 4: Produtor-Consumidor multithreaded com memoria compartilhada e semaforos.
//
// Uso:
//   ./prodcons_sem <N> <Np> <Nc> [opcoes]
//     N   tamanho do vetor compartilhado (buffer circular)
//     Np  numero de threads produtoras
//     Nc  numero de threads consumidoras
//
//   Opcoes:
//     -m <M>            quantidade de numeros a consumir (padrao 100000)
//     --silencioso      nao imprime o resultado de cada numero (usado no estudo
//                       de caso, para que o custo de E/S nao domine o tempo)
//     --ocupacao <arq>  registra a ocupacao do buffer apos cada operacao e
//                       grava o historico em <arq>, um valor por linha
//
// Sincronizacao (esquema classico visto em aula):
//   vazias  - semaforo contador, inicia em N: quantas posicoes livres existem
//   cheias  - semaforo contador, inicia em 0: quantas posicoes ocupadas existem
//   mutex   - semaforo binario, inicia em 1: exclusao mutua sobre o vetor
//
// A ordem dos acquire() importa: o semaforo contador SEMPRE vem antes do mutex.
// Inverter as duas linhas permite que uma thread durma segurando o mutex,
// travando todas as outras -> deadlock.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kValorMaximo = 10'000'000;  // produtor sorteia em [1, 10^7]

// Teste de primalidade por divisao ate a raiz, pulando multiplos de 2 e 3.
// E aqui que o consumidor gasta trabalho de CPU de verdade.
bool eh_primo(int n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Memoria compartilhada protegida por semaforos.
// O vetor e usado como buffer circular; 'vazias' e 'cheias' fazem o bloqueio
// das threads quando ele esta cheio ou vazio, e 'mutex' serializa o acesso.
class BufferCompartilhado {
public:
    BufferCompartilhado(std::size_t capacidade, bool registrar_ocupacao,
                        std::size_t reserva_historico)
        : buf_(capacidade),
          capacidade_(capacidade),
          vazias_(static_cast<std::ptrdiff_t>(capacidade)),
          cheias_(0),
          registrar_(registrar_ocupacao) {
        if (registrar_) historico_.reserve(reserva_historico);
    }

    void produzir(int valor) {
        vazias_.acquire();   // espera ate existir posicao livre
        mutex_.acquire();    // entra na secao critica
        buf_[fim_] = valor;
        fim_ = (fim_ + 1) % capacidade_;
        ++ocupadas_;
        if (registrar_) historico_.push_back(static_cast<std::uint32_t>(ocupadas_));
        mutex_.release();
        cheias_.release();   // avisa que ha mais um item disponivel
    }

    int consumir() {
        cheias_.acquire();   // espera ate existir item produzido
        mutex_.acquire();
        int valor = buf_[inicio_];
        inicio_ = (inicio_ + 1) % capacidade_;
        --ocupadas_;
        if (registrar_) historico_.push_back(static_cast<std::uint32_t>(ocupadas_));
        mutex_.release();
        vazias_.release();   // libera a posicao para os produtores
        return valor;
    }

    const std::vector<std::uint32_t>& historico() const { return historico_; }

private:
    std::vector<int> buf_;
    std::size_t capacidade_;
    std::size_t inicio_ = 0;    // proxima posicao a ser lida
    std::size_t fim_ = 0;       // proxima posicao a ser escrita
    std::size_t ocupadas_ = 0;  // so para o historico; quem conta de verdade
                                // sao os semaforos

    std::counting_semaphore<> vazias_;
    std::counting_semaphore<> cheias_;
    std::binary_semaphore mutex_{1};

    bool registrar_;
    std::vector<std::uint32_t> historico_;
};

// Protege apenas a impressao no terminal, para que linhas de threads
// diferentes nao se misturem. Nao participa da sincronizacao do buffer.
std::mutex g_mutex_saida;

void uso(const char* prog) {
    std::fprintf(stderr,
                 "Uso: %s <N> <Np> <Nc> [-m M] [--silencioso] [--ocupacao ARQ]\n"
                 "  ex.: %s 100 2 4\n"
                 "  ex.: %s 10 1 8 --silencioso\n",
                 prog, prog, prog);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 4) {
        uso(argv[0]);
        return EXIT_FAILURE;
    }

    long N = std::strtol(argv[1], nullptr, 10);
    long Np = std::strtol(argv[2], nullptr, 10);
    long Nc = std::strtol(argv[3], nullptr, 10);
    long M = 100000;
    bool silencioso = false;
    std::string arquivo_ocupacao;

    for (int i = 4; i < argc; ++i) {
        std::string opt = argv[i];
        if (opt == "--silencioso") {
            silencioso = true;
        } else if (opt == "-m" && i + 1 < argc) {
            M = std::strtol(argv[++i], nullptr, 10);
        } else if (opt == "--ocupacao" && i + 1 < argc) {
            arquivo_ocupacao = argv[++i];
        } else {
            std::fprintf(stderr, "[erro] opcao desconhecida: %s\n", opt.c_str());
            uso(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (N <= 0 || Np <= 0 || Nc <= 0 || M <= 0) {
        std::fprintf(stderr, "[erro] N, Np, Nc e M devem ser positivos.\n");
        return EXIT_FAILURE;
    }

    const bool registrar = !arquivo_ocupacao.empty();
    BufferCompartilhado buffer(static_cast<std::size_t>(N), registrar,
                               static_cast<std::size_t>(2 * M + 16));

    // Contadores globais de trabalho reservado.
    // Cada thread "pega um numero de senha" com fetch_add antes de trabalhar.
    // Isso garante que exatamente M itens sejam produzidos e exatamente M
    // sejam consumidos, o que por sua vez garante que nenhuma thread fique
    // bloqueada para sempre num acquire() no fim da execucao.
    std::atomic<long> reservados_producao{0};
    std::atomic<long> reservados_consumo{0};
    std::atomic<long> total_primos{0};

    auto inicio = std::chrono::steady_clock::now();

    std::vector<std::thread> produtores;
    produtores.reserve(static_cast<std::size_t>(Np));
    for (long p = 0; p < Np; ++p) {
        produtores.emplace_back([&, p] {
            std::mt19937 rng(static_cast<std::mt19937::result_type>(
                std::random_device{}() ^ (0x9E3779B9u * static_cast<unsigned>(p))));
            std::uniform_int_distribution<int> dist(1, kValorMaximo);
            for (;;) {
                if (reservados_producao.fetch_add(1, std::memory_order_relaxed) >= M)
                    break;
                buffer.produzir(dist(rng));
            }
        });
    }

    std::vector<std::thread> consumidores;
    consumidores.reserve(static_cast<std::size_t>(Nc));
    for (long c = 0; c < Nc; ++c) {
        consumidores.emplace_back([&, c] {
            long primos_locais = 0;
            for (;;) {
                if (reservados_consumo.fetch_add(1, std::memory_order_relaxed) >= M)
                    break;
                int valor = buffer.consumir();
                bool primo = eh_primo(valor);
                if (primo) ++primos_locais;
                if (!silencioso) {
                    std::lock_guard<std::mutex> lock(g_mutex_saida);
                    std::printf("[consumidor %ld] %d %s\n", c, valor,
                                primo ? "eh PRIMO" : "NAO eh primo");
                }
            }
            total_primos.fetch_add(primos_locais, std::memory_order_relaxed);
        });
    }

    for (auto& t : produtores) t.join();
    for (auto& t : consumidores) t.join();

    auto fim = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(fim - inicio).count();

    if (registrar) {
        std::ofstream out(arquivo_ocupacao);
        if (!out) {
            std::fprintf(stderr, "[erro] nao foi possivel escrever em %s\n",
                         arquivo_ocupacao.c_str());
        } else {
            for (std::uint32_t v : buffer.historico()) out << v << '\n';
        }
    }

    // Linha final em formato fixo, para os scripts do estudo de caso.
    std::printf("N=%ld Np=%ld Nc=%ld M=%ld primos=%ld TEMPO_MS=%.3f\n", N, Np, Nc, M,
                total_primos.load(), ms);
    return EXIT_SUCCESS;
}
