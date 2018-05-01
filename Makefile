#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: libcthread cthread.o

libcthread: cthread.o #dependências para a regra1
	ar crs $(LIB_DIR)/libcthread.a $(BIN_DIR)/cthread.o $(BIN_DIR)/cdata.o $(BIN_DIR)/support.o

cthread.o: $(SRC_DIR)cthread.c #dependências para a regra2
	$(CC) -c $(SRC_DIR)cthread.c $(BIN_DIR)cthread.o

clean:
	rm -rf $(LIB_DIR)/libcthread.a
	rm -rf $(BIN_DIR)/cthread.o
