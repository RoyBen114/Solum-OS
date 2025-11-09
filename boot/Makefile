S_SRC = $(wildcard *.s)
OBJ = $(S_SRC:.s=.o)

all:
	nasm -f elf $(S_SRC) -o $(OBJ)

clean:
	rm -f $(OBJ)

.PHONY: clean