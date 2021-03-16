CC=gcc
CFLAGS=-g -Wall -fopenmp -O3
LDFLAGS=-lm
SOURCE=$(wildcard *.c)
OBJ=$(SOURCE:.c=.o)
EXEC=$(SOURCE:.c=)

all: $(EXEC)

clean:
	rm -rf $(OBJ) $(EXEC) *~ *aux *.dSYM

