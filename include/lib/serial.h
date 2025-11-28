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

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char *str);
void serial_printk(const char *format, ...);
char serial_read_char(void);
bool serial_read_line(char *buffer, size_t buffer_size);
int serial_scank(const char *format, ...);

#endif