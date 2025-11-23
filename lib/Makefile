C_SRC = $(wildcard *.c)
O_OBJ = $(patsubst %.c, %.o, $(C_SRC))
all:
	gcc -c -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -mno-red-zone -ffreestanding $(C_SRC) 

clean:
	rm -f $(O_OBJ)

.PHONY: clean all