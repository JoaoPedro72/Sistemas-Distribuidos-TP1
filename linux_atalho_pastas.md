# Pasta
cd /mnt/c/Arkivos/codigos/"TP1 - SD"

# Sinais
cd Sinais
## Compilar
g++ "Enviar Linux.cpp" -o "Enviar Linux.exe"

g++ "Receber Linux.cpp" -o "Receber Linux.exe"

./"Enviar Linux.exe" PID SINAL

Exemplo: ./Enviar Linux.exe 12345 10

./"Receber Linux.exe" busy|blocking

busy - espera ocupada - fica rodando um for ate receber resposta
blocking - bloqueio - espera um sinal de evento para rodar, não comsome estenso tempo de CPU

# Pipes
cd Pipes

./mainLinux.exe <quantidade>

exemplo
./mainLinux.exe 8