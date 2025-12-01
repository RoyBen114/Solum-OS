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

#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <stdarg.h>

typedef enum {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GREY = 7,
    DARK_GREY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    YELLOW = 14,   
    WHITE = 15
} vga_color_t;

void screen_scroll_once(void);
void move_cursor_by_xy(uint8_t x, uint8_t y);
void clear_screen(void);
void vga_init(void);
void vga_putc(char c);
void vga_putc_color(char c, vga_color_t back, vga_color_t fore);
void vga_puts(const char *str);
void vga_puts_color(const char *str, vga_color_t back, vga_color_t fore);
void vga_printk(const char *format, ...);
void vga_printk_color(vga_color_t back, vga_color_t fore, const char *format, ...);

#endif