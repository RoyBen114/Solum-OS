TARGET = Solum.iso
OBJ = boot/boot.o kernel/kernel.o
KELF = kernel.elf

$(TARGET): $(KELF)
	cp grub.cfg isodir/boot/grub/grub.cfg
	cp $(KELF) isodir/boot/$(KELF)
	grub-mkrescue -o Solum.iso isodir/

$(KELF): $(OBJ)
	ld -m elf_i386 -n -T linker.ld -o $(KELF) $(OBJ)

$(OBJ):
	make -C boot all
	make -C kernel

clean: 
	make -C boot clean
	make -C kernel clean
	rm -f $(TARGET)
	rm -f $(KELF)

run:
	make clean
	make
	qemu-system-x86_64 -cdrom $(TARGET) -m 1G

.PHONY: run clean