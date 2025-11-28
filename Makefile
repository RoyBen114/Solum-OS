#
# Copyright (C) 2025 Roy Roy123ty@hotmail.com
# 
# This file is part of Solum OS
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TARGET = Solum.iso
KELF = kernel.elf
LINKSCR = linker.ld
BUILD ?= release
INCDIR := $(CURDIR)/include

ifeq ($(BUILD),release)
CFLAGS := -c -O2 -I$(INCDIR) -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -mno-red-zone -ffreestanding
else ifeq ($(BUILD),debug)
CFLAGS := -g -c -O0 -I$(INCDIR) -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -mno-red-zone -ffreestanding
endif

BOOT_OBJ = boot/boot.o
KERN_OBJ = kernel/kernel.o
INFO_OBJ = boot/info.o
BOOT_S = boot/boot.s
KERN_C = kernel/kernel.c
INFO_C = boot/info.c
LIB_C = $(shell find lib/ -name "*.c")
LIB_OBJ = $(patsubst %.c, %.o, $(LIB_C))

$(TARGET): $(KELF) grub.cfg
	cp grub.cfg ISODir/boot/grub/grub.cfg
	cp $(KELF) ISODir/SolumOS/$(KELF)
	grub-mkrescue -o Solum.iso ISODir/

$(KELF): $(BOOT_OBJ) $(KERN_OBJ) $(INFO_OBJ) $(LIB_OBJ) $(LINKSCR)
	ld -n -T $(LINKSCR) -o $(KELF) $(BOOT_OBJ) $(KERN_OBJ) $(INFO_OBJ) $(LIB_OBJ)

$(BOOT_OBJ): $(BOOT_S)
	make -C boot BOOT_O

$(INFO_OBJ): $(INFO_C)
	make -C boot INFO_O CFLAGS="$(CFLAGS)" 

$(KERN_OBJ): $(KERN_C)
	make -C kernel KERN_O CFLAGS="$(CFLAGS)" 

$(LIB_OBJ): $(LIB_C)
	make -C lib all CFLAGS="$(CFLAGS)" 

clean: 
	make -C boot clean
	make -C kernel clean
	make -C lib clean
	rm -f $(TARGET)
	rm -f $(KELF)

debug_B:
	make BUILD=debug
	qemu-system-x86_64 -cdrom $(TARGET) -m 1G -serial stdio -S -s

debug_U:
	make BUILD=debug
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom $(TARGET) -m 1G -serial stdio -S -s

.PHONY: debug_B debug_U