# Makefile - Cadastro de Loja em C
CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11
SRC     = src/cadastro_loja.c
BIN     = build/cadastro_loja

.PHONY: all run test clean

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

run: all
	./$(BIN)

test: all
	bash tests/test.sh

clean:
	rm -rf build produtos.txt produtos_temp.txt produtos.csv
