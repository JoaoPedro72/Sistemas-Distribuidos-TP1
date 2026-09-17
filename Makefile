# TP1 - Sistemas Distribuidos - CEFET-MG
# Compila as versoes Linux das tres partes em bin/.
#
#   make            compila tudo
#   make sinais     parte 2
#   make pipes      parte 3
#   make semaforos  parte 4
#   make limpar     remove bin/
#
# As versoes Windows da parte 2 sao compiladas por Sinais/compileWin.bat.

CXX      = g++
CXXFLAGS = -Wall -Wextra -O2
BIN      = bin

all: sinais pipes semaforos

sinais:    $(BIN)/enviar $(BIN)/receber
pipes:     $(BIN)/pipes
semaforos: $(BIN)/prodcons_sem

$(BIN):
	mkdir -p $(BIN)

$(BIN)/enviar: Sinais/Enviar\ Linux.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -std=c++17 -o $@ "$<"

$(BIN)/receber: Sinais/Receber\ Linux.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -std=c++17 -o $@ "$<"

$(BIN)/pipes: Pipes/main\ linux.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -std=c++17 -o $@ "$<"

$(BIN)/prodcons_sem: Semaforos/prodcons_sem.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -std=c++17 -pthread -o $@ $<

limpar:
	rm -rf $(BIN)

.PHONY: all sinais pipes semaforos limpar
