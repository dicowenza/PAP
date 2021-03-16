.PHONY: default clean

SOURCE	= $(wildcard *.c)
EXEC	= $(SOURCE:.c=)

CC	= gcc
CFLAGS	= -fopenmp -O3
LDFLAG	= -fopenmp

default: $(EXEC)

clean:
	rm -f $(EXEC)
