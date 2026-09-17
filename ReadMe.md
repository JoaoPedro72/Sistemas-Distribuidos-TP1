# Trabalho Prático 1
## Sistemas Distribuídos - CEFET - MG
## Professora: Michelle Hanne

## Alunos:
- João Pedro da Silva Alves
- Augusto Soares
- Gabriel Jovenal

## Estrutura

| Pasta | Parte | Conteúdo |
|---|---|---|
| `Sinais/` | 2 | `Enviar Linux.cpp`, `Receber Linux.cpp` (+ versões Windows, que simulam sinais com Events) |
| `Pipes/` | 3 | `main linux.cpp` (mesma lógica em C: `main linux.c`) |
| `Semaforos/` | 4 | `prodcons_sem.cpp`, `scripts/`, `resultados/` |

## Compilação

```bash
make            # compila as três partes em bin/
make limpar     # remove bin/
```

Tudo compila com C++17; a parte 4 usa semáforos POSIX (`sem_t`). As versões Windows da parte 2 saem de `Sinais/compileWin.bat`.

## Execução

### Parte 2 — Sinais

```bash
./bin/receber blocking          # espera bloqueante (ou: busy)
./bin/enviar <PID> <SINAL>      # ex.: ./bin/enviar 4231 10
```

`receber` captura SIGUSR1 (10), SIGUSR2 (12) e SIGINT (2); SIGINT encerra o processo. `kill -USR1 <PID>` também funciona. `enviar` retorna erro se o PID não existir.

### Parte 3 — Pipes

```bash
./bin/pipes <quantidade>        # ex.: ./bin/pipes 1000
```

O pai produz e o filho consome pelo pipe anônimo; cada número vai como string de 20 bytes e o 0 final encerra o consumidor.

### Parte 4 — Produtor-Consumidor com semáforos

```bash
./bin/prodcons_sem <N> <Np> <Nc> [-m M] [--silencioso] [--ocupacao ARQ]
```

| Parâmetro | Significado |
|---|---|
| `N` | tamanho do vetor compartilhado (buffer circular) |
| `Np` / `Nc` | número de threads produtoras / consumidoras |
| `-m M` | quantos números consumir (padrão `100000`) |
| `--silencioso` | não imprime o resultado de cada número |
| `--ocupacao ARQ` | grava a ocupação do buffer após cada operação |

```bash
./bin/prodcons_sem 10 2 4 -m 20         # demonstração curta, com impressão
./bin/prodcons_sem 100 1 8 --silencioso
```

Cada execução termina com uma linha em formato fixo, lida pelos scripts:

```
N=100 Np=2 Nc=4 M=100000 primos=6820 TEMPO_MS=59.335
```

**Sincronização:** três semáforos — `vazias` (inicia em N, conta posições livres), `cheias` (inicia em 0, conta posições ocupadas) e `mutex` (binário, serializa o acesso ao vetor). O semáforo contador é sempre adquirido antes do mutex; na ordem inversa uma thread dormiria segurando o mutex e travaria as demais. O término usa dois contadores atômicos, que garantem exatamente M produções e M consumos — assim nenhuma thread fica presa num `acquire()` no fim.

## Estudo de caso (parte 4)

```bash
bash Semaforos/scripts/estudo_caso.sh        # 4 valores de N x 7 combinações x 10 execuções
python3 Semaforos/scripts/gera_graficos.py   # gráficos e tabela
```

Saídas em `Semaforos/resultados/`:

| Arquivo | Conteúdo |
|---|---|
| `tempos.csv` | tempo de cada execução individual |
| `tempos_medios.csv` | média e desvio padrão por cenário |
| `tempo_medio.png` | tempo médio × combinação de threads, uma curva por N |
| `ocupacao.png` | ocupação do buffer ao longo do tempo, 28 cenários |
| `ocupacao/*.txt` | histórico bruto de ocupação (não versionado) |

---

# Enunciado

## 1 - Objetivos

O objetivo deste trabalho é se familiarizar com os principais mecanismos de
IPC (Interprocess Communication) baseados em troca de mensagens, threads
e mecanismos de sincronização. Para cada parte, você deve desenvolver um
programa na linguagem de programação de sua preferência mas que tenha
suporte a threads e mecanismos de sincronização, como instruções atômicas
e semáforos. A sugestão é utilizar C ou C++, tendo em vista a proximidade
das bibliotecas destas linguagens com o sistema operacional, oferecendo ao
desenvolvedor (você) um maior controle.

Além da implementação, você deve testar seu programa, rodando os estudos de casos. Você deve preparar um relatório, com no máximo 5 páginas,
com as decisões de projeto e implementação das funcionalidades especificadas, assim como a avaliação dos estudos de caso. O relatório deve conter a
URL para o código-fonte da sua implementação. O trabalho pode ser realizado em dupla.

## 2 - Sinais
Nesta tarefa você deve escrever dois programas distintos.

O primeiro programa deve ser capaz de enviar um sinal a qualquer outro
processo. Este programa recebe como parâmetros o número do processo
destino e o sinal que deve ser enviado. Seu programa deve verificar se o
processo indicado pelo parâmetro existe, e retornar um erro em caso caso
negativo. Caso positivo, seu programa deve enviar o sinal indicado.

O segundo programa deve ser capaz de receber alguns sinais especıficos.
Para isto, você precisa definir signal handlers. Seu programa deve capturar e reagir a três sinais diferentes (de sua escolha), ou mais, imprimindo
no terminal uma mensagem diferente para cada sinal. Além disso, um dos
sinais sendo capturados deve terminar a execução do programa, ou seja, sua
signal handler deve terminar o processo. Repare que após estipular as signal
handlers seu programa fica aguardando a chegada de sinais. Você deve implementar duas formas de esperar, busy wait e blocking wait (passado como
parâmetro para o programa).

Descubra como implementar cada um destas formas de fazer um processo
esperar! Teste seus programas fazendo com que um envie sinais para o outro.
Use também o programa kill para enviar sinais para o seu segundo programa!
É possível usar bibliotecas como <signal.h> em C.

## 3 - Pipes
Implemente o programa Produtor-Consumidor como vimos em aula com dois
processos que utilizam pipes (anonymous pipes, para ser mais preciso) para
fazer a comunicação.

O programa produtor deve gerar números inteiros aleatórios e crescentes,
da seguinte forma:

Ni = Ni−1 + ∆, N0 = 1, ∆ ∈ [1, 100].

O programa consumidor deve receber o número e verificar se o mesmo
é primo, imprimindo o resultado no terminal. Seu programa deve primeiramente criar um pipe e depois fazer um fork() para duplicar o processo, de
forma que os dois processos (pai e filho) tenham as duas respectivas pontas
do pipe (write end e read end).

O processo consumidor deve terminar quando receber o número 0. O programa produtor tem como parâmetro o número de números a serem gerados
(ex.: 1000), depois do qual o número zero é enviado, e o produtor termina
sua execução.
Cuidado com a representação numérica ao escrever no pipe!
Dica: converta o número para uma string de tamanho fixo, por exemplo,
20 bytes. Escreva e leia do pipe este mesmo número de bytes para cada
mensagem.

Teste o seu programa mostrando seu funcionamento para alguns casos.

## 4 - Produtor-Consumidor com Semáforos
Implemente um programa Produtor-Consumidor multithreaded com memória compartilhada. Assuma que a memória compartilhada é um vetor de
números inteiros de tamanho N. O número de threads do tipo produtor
e consumidor são parâmetros do programa dados por Np e Nc, respectivamente. A thread produtor deve gerar números inteiros aleatórios entre 1 e
107
e colocar o número em uma posição livre da memória compartilhada. A
thread consumidor deve retirar um número produzido por um produtor da
memória compartilhada, liberar a posição do vetor, e verificar se o mesmo é
primo, imprimindo o resultado no terminal.

Repare que a memória compartilhada será escrita e lida por várias threads, então o acesso deve ser serializado, evitando efeitos indesejáveis da
condição de corrida. Utilize semáforos para serializar o acesso à memória
compartilhada. Repare ainda que quando a memória compartilhada estiver
cheia ou vazia, as threads produtor ou consumidor devem aguardar bloqueadas, respectivamente. Ou seja, uma thread produtor aguarda até que haja
uma posição de memória livre, e uma thread consumidor aguarda até que
haja uma posição de memória ocupada. Utilize semáforos contadores para
esta coordenação, como visto em aula.

Para o estudo de caso, considere que o programa termina sua execução
após o consumidor processar M = 105 números. Considere ainda os valores
N = 1, 10, 100, 1000, com os seguintes combinações de número de threads
produtor/consumidor:

(Np, Nc) ∈ {(1, 1),(1, 2),(1, 4),(1, 8),(2, 1),(4, 1),(8, 1)}.

Para cada combinação de parâmetros, obtenha o tempo de execução do
seu programa, rodando o programa 10 vezes para calcular o tempo médio
de execução. Apresente um gráfico mostrando o tempo médio de execução
em função do número de threads produtor/consumidor para cada valor de N
(cada N deve ser uma curva no gráfico). Analise o comportamento observado.

Para cada cenário, trace um gráfico representativo com a ocupação do
buffer compartilhado ao longo do tempo. Para gerar o gráfico, utilize um
vetor que armazena a ocupação do buffer após cada operação de produção
ou consumo. Ao final, use o vetor para gerar o gráfico (ou arquivo).

O que você pode concluir em cada um dos casos?