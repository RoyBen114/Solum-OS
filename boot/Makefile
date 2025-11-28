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

S_SRC = boot.s
C_SRC = info.c
BOOT_OBJ = boot.o
INFO_OBJ = info.o

all:
	make BOOT_O
	make INFO_O

BOOT_O:
	nasm -f elf64 $(S_SRC) -o $(BOOT_OBJ)

INFO_O:
	gcc -c -I../include -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -mno-red-zone -ffreestanding $(C_SRC) $(INFO_OBJ)

clean:
	rm -f $(BOOT_OBJ)
	rm -f $(INFO_OBJ)

.PHONY: clean BOOT_O INFO_O all