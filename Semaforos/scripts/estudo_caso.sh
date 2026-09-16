#!/bin/bash
# Estudo de caso da Parte 4.
#   N  in {1, 10, 100, 1000}
#   (Np, Nc) in {(1,1),(1,2),(1,4),(1,8),(2,1),(4,1),(8,1)}
#   10 repeticoes por combinacao (ajustavel por REP=n)
# Gera:
#   resultados/tempos.csv            -> tempos de cada execucao
#   resultados/ocupacao/*.txt        -> ocupacao do buffer (1 execucao por cenario)
set -u
cd "$(dirname "$0")/.."

BIN=${BIN:-../bin/prodcons_sem}
REP=${REP:-10}
OUT=resultados/tempos.csv
COMBOS="1:1 1:2 1:4 1:8 2:1 4:1 8:1"

mkdir -p resultados/ocupacao
echo "N,Np,Nc,rep,tempo_ms" > "$OUT"

for N in 1 10 100 1000; do
  for c in $COMBOS; do
    NP=${c%:*}; NC=${c#*:}
    for r in $(seq 1 "$REP"); do
      LINHA=$($BIN "$N" "$NP" "$NC" --silencioso)
      TEMPO=${LINHA##*TEMPO_MS=}
      echo "$N,$NP,$NC,$r,$TEMPO" >> "$OUT"
    done
    # execucao extra, so para registrar a ocupacao do buffer ao longo do tempo
    $BIN "$N" "$NP" "$NC" --silencioso \
         --ocupacao "resultados/ocupacao/N${N}_P${NP}_C${NC}.txt" > /dev/null
    echo "ok  N=$N  Np=$NP  Nc=$NC"
  done
done

echo "--- concluido. dados em $OUT ---"
