/*
 * Copyright (C) 2025 Roy Roy123ty@hotmail.com
 * 
 * This file is part of Solum OS
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TTY_H
#define TTY_H

#include <stddef.h>
#include <kernel/screen.h>

void tty_init(void);
size_t tty_write(int fd, const char *buf, size_t len, vga_color_t fore, vga_color_t back);
size_t tty_read(char *dest, size_t len);
void tty_flush(void);

#endif
