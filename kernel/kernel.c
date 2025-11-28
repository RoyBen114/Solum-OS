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

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <boot/info.h>
#include <lib/serial.h>
#include <lib/screen.h>

void kernel_main() 
{
    serial_init();
    vga_init();
    parse_mb_info();

    vga_printk("Welcome to Solum OS!\n");
    serial_printk("Welcome to Solum OS!\n");
    vga_printk("Version (a0.01)\n");
    serial_printk("Version (a0.01)\n");
    vga_printk("By Roy - 2025\n");
    serial_printk("By Roy - 2025\n");
}