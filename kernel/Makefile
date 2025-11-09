C_SRC = $(wildcard *.c)
OBJ = $(C_SRC:.c=.o)

all:
	gcc -c -fno-builtin -ffreestanding -nostdlib -m32 $(C_SRC) -o $(OBJ)

clean:
	rm -f $(OBJ)

.PHONY: clean