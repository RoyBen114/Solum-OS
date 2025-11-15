S_SRC = boot.s
BOOT_OBJ = boot.o

all:
	nasm -f elf64 $(S_SRC) -o $(BOOT_OBJ)

clean:
	rm -f $(BOOT_OBJ)

.PHONY: clean