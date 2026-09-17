#!/usr/bin/env python3
"""Gera os graficos do estudo de caso da Parte 4.

Entradas:
    resultados/tempos.csv          (produzido por scripts/estudo_caso.sh)
    resultados/ocupacao/*.txt      (idem)

Saidas:
    resultados/tempo_medio.png     tempo medio x combinacao de threads, 1 curva por N
    resultados/ocupacao.png        ocupacao do buffer ao longo do tempo (28 cenarios)
    resultados/tempos_medios.csv   tabela media/desvio, para o relatorio
"""
import csv
import os
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(RAIZ, "resultados")

# Paleta de referencia validada (slots categoricos 1..4).
SERIES = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100"]
SURFACE = "#fcfcfb"
INK = "#0b0b0b"
INK2 = "#52514e"
GRID = "#e3e2de"

VALORES_N = [1, 10, 100, 1000]
COMBOS = [(1, 1), (1, 2), (1, 4), (1, 8), (2, 1), (4, 1), (8, 1)]
ROTULOS = ["{}P/{}C".format(p, c) for p, c in COMBOS]


def estilo(ax):
    """Grade e eixos recessivos; nada de tinta forte fora dos dados."""
    ax.set_facecolor(SURFACE)
    for lado in ("top", "right"):
        ax.spines[lado].set_visible(False)
    for lado in ("left", "bottom"):
        ax.spines[lado].set_color(GRID)
    ax.tick_params(colors=INK2, labelsize=9, length=3, width=0.8)
    ax.grid(True, color=GRID, linewidth=0.8, alpha=0.9)
    ax.set_axisbelow(True)


def carrega_tempos():
    dados = defaultdict(list)
    with open(os.path.join(RES, "tempos.csv"), newline="", encoding="utf-8") as f:
        for linha in csv.DictReader(f):
            chave = (int(linha["N"]), int(linha["Np"]), int(linha["Nc"]))
            dados[chave].append(float(linha["tempo_ms"]))
    return dados


def grafico_tempos(dados):
    fig, ax = plt.subplots(figsize=(9.5, 5.6), dpi=200)
    fig.patch.set_facecolor(SURFACE)
    estilo(ax)

    x = np.arange(len(COMBOS))
    curvas, desvios_por_N = [], []
    for N in VALORES_N:
        medias, desvios = [], []
        for combo in COMBOS:
            amostras = dados.get((N,) + combo, [])
            medias.append(np.mean(amostras) if amostras else np.nan)
            desvios.append(np.std(amostras) if amostras else 0.0)
        curvas.append(np.array(medias))
        desvios_por_N.append(np.array(desvios))
    curvas = np.array(curvas)

    # Onde ancorar o rotulo de cada curva: no ponto em que ela fica mais longe
    # das demais, medindo em escala log (a do eixo). Ancorar sempre no ultimo
    # ponto empilha os rotulos de N=10, 100 e 1000 uns sobre os outros, porque
    # as tres curvas convergem na direita.
    with np.errstate(invalid="ignore"):
        log = np.log10(curvas)
    ancora = []
    for i in range(len(VALORES_N)):
        outras = np.delete(log, i, axis=0)
        dist = np.nanmin(np.abs(outras - log[i]), axis=0)
        ancora.append(int(np.nanargmax(dist)))

    for i, N in enumerate(VALORES_N):
        cor = SERIES[i]
        ax.errorbar(x, curvas[i], yerr=desvios_por_N[i], color=cor, linewidth=2.0,
                    marker="o", markersize=6, markeredgecolor=SURFACE,
                    markeredgewidth=1.2, elinewidth=0.9, capsize=2.5,
                    ecolor=cor, alpha=0.95, label="N = {}".format(N), zorder=3)
        # Rotulo direto na curva: tres dos quatro tons ficam abaixo de 3:1 de
        # contraste com o fundo, entao a identidade da serie nao pode depender
        # apenas da cor.
        j = ancora[i]
        if not np.isnan(curvas[i][j]):
            ax.annotate("N = {}".format(N), xy=(x[j], curvas[i][j]),
                        xytext=(0, 10), textcoords="offset points",
                        va="bottom", ha="center", fontsize=9, color=INK,
                        zorder=4)

    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(ROTULOS)
    ax.set_xlim(-0.35, len(COMBOS) - 1 + 0.35)
    ax.set_xlabel("combinação produtor/consumidor (Np/Nc)",
                  color=INK2, fontsize=10)
    ax.set_ylabel("tempo médio de execução (ms, escala log)",
                  color=INK2, fontsize=10)
    ax.set_title("Tempo médio de execução por combinação de "
                 "threads\nM = 100.000 números consumidos, 10 execuções "
                 "por ponto (barras = desvio padrão)",
                 color=INK, fontsize=12, loc="left", pad=28)
    leg = ax.legend(frameon=False, fontsize=9, ncol=4, loc="lower left",
                    bbox_to_anchor=(0, 1.0), borderaxespad=0)
    for t in leg.get_texts():
        t.set_color(INK2)

    fig.tight_layout()
    saida = os.path.join(RES, "tempo_medio.png")
    fig.savefig(saida, facecolor=SURFACE)
    plt.close(fig)
    print("gerado:", saida)


def grafico_ocupacao():
    dir_oc = os.path.join(RES, "ocupacao")
    fig, axes = plt.subplots(len(VALORES_N), len(COMBOS),
                             figsize=(17.5, 9.5), dpi=150)
    fig.patch.set_facecolor(SURFACE)

    for li, N in enumerate(VALORES_N):
        for co, (p, c) in enumerate(COMBOS):
            ax = axes[li][co]
            estilo(ax)
            caminho = os.path.join(dir_oc, "N{}_P{}_C{}.txt".format(N, p, c))
            if os.path.exists(caminho):
                with open(caminho, encoding="utf-8") as f:
                    v = np.array(f.read().split(), dtype=np.int32)
                # Envelope min/max por bloco. Subamostrar pegando 1 ponto a
                # cada k esconderia a oscilacao rapida: com N=1 o buffer
                # alterna 0<->1 a cada operacao e o grafico viraria uma reta
                # em 1. A faixa mostra a variacao real dentro do bloco; a
                # linha mostra a ocupacao media.
                blocos = 600
                if len(v) >= blocos * 2:
                    corte = (len(v) // blocos) * blocos
                    janela = v[:corte].reshape(blocos, -1)
                    xs = np.arange(blocos) * (corte / blocos)
                    ax.fill_between(xs, janela.min(axis=1), janela.max(axis=1),
                                    color=SERIES[0], alpha=0.30, linewidth=0)
                    ax.plot(xs, janela.mean(axis=1), color=SERIES[0],
                            linewidth=1.0)
                else:
                    ax.plot(np.arange(len(v)), v, color=SERIES[0], linewidth=0.9)
            ax.set_ylim(-0.05 * N, N * 1.05)
            ax.set_title("N={}  {}P/{}C".format(N, p, c), fontsize=8.5,
                         color=INK, pad=4)
            ax.tick_params(labelsize=7)
            if co == 0:
                ax.set_ylabel("ocupação", fontsize=8, color=INK2)
            if li == len(VALORES_N) - 1:
                ax.set_xlabel("operações", fontsize=8, color=INK2)

    fig.suptitle("Ocupação do buffer compartilhado ao longo da "
                 "execução  —  faixa = mínimo–máximo "
                 "por bloco, linha = média",
                 color=INK, fontsize=13, x=0.006, ha="left", y=0.995)
    fig.tight_layout(rect=[0, 0, 1, 0.972])
    saida = os.path.join(RES, "ocupacao.png")
    fig.savefig(saida, facecolor=SURFACE)
    plt.close(fig)
    print("gerado:", saida)


def tabela(dados):
    saida = os.path.join(RES, "tempos_medios.csv")
    with open(saida, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["N", "Np", "Nc", "media_ms", "desvio_ms", "execucoes"])
        for N in VALORES_N:
            for p, c in COMBOS:
                a = dados.get((N, p, c), [])
                if a:
                    w.writerow([N, p, c, "{:.1f}".format(np.mean(a)),
                                "{:.1f}".format(np.std(a)), len(a)])
    print("gerado:", saida)


def main():
    dados = carrega_tempos()
    grafico_tempos(dados)
    grafico_ocupacao()
    tabela(dados)


if __name__ == "__main__":
    main()
