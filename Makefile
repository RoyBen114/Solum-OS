TARGET = Solum.iso
KELF = kernel.elf
LINKSCR = linker.ld
BOOT_OBJ = boot/boot.o
KERN_OBJ = kernel/kernel.o
S_SRC = boot/boot.s
C_SRC = kernel/kernel.c

$(TARGET): $(KELF) grub.cfg
	cp grub.cfg ISODir/boot/grub/grub.cfg
	cp $(KELF) ISODir/SolumOS/$(KELF)
	grub-mkrescue -o Solum.iso ISODir/

$(KELF): $(BOOT_OBJ) $(KERN_OBJ) $(LINKSCR)
	ld -n -T $(LINKSCR) -o $(KELF) $(BOOT_OBJ) $(KERN_OBJ)

$(BOOT_OBJ): $(S_SRC)
	make -C boot all

$(KERN_OBJ): $(C_SRC)
	make -C kernel all

clean: 
	make -C boot clean
	make -C kernel clean
	rm -f $(TARGET)
	rm -f $(KELF)

run:
	make
	qemu-system-x86_64 -cdrom $(TARGET) -m 1G

.PHONY: run clean