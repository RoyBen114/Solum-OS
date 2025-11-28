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
#include <stdbool.h>
#include <lib/screen.h>
#include <lib/string.h>
#include <lib/port.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define TAB_LENGTH 4

static uint16_t *video_memory = (uint16_t *)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static void move_cursor(void)
{
    uint16_t cursor_location = cursor_y * SCREEN_WIDTH + cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, cursor_location >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, cursor_location & 0xFF);
}

void move_cursor_by_xy(uint8_t x, uint8_t y)
{
    if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
    if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
    
    cursor_x = x;
    cursor_y = y;
    move_cursor();
}

void clear_screen(void)
{
    cursor_x = 0;
    cursor_y = 0;
    
    const uint16_t blank = ' ' | (WHITE << 8);
    for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_memory[i] = blank;
    }
    move_cursor();
}

void screen_scroll_once(void)
{
    // Move all lines up by one
    for (size_t j = 0; j < SCREEN_HEIGHT - 1; j++) {
        for (size_t i = 0; i < SCREEN_WIDTH; i++) {
            video_memory[j * SCREEN_WIDTH + i] = video_memory[(j + 1) * SCREEN_WIDTH + i];
        }
    }
    
    // Clear the last line
    const uint16_t blank = ' ' | (WHITE << 8);
    for (size_t i = 0; i < SCREEN_WIDTH; i++) {
        video_memory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i] = blank;
    }
    
    if (cursor_y > 0) {
        cursor_y--;
    }
    move_cursor();
}

static void vga_putc_raw(char c, vga_color_t back, vga_color_t fore)
{
    if (c == '\0') c = ' '; // Replace null with space
    
    uint16_t attribute = (back << 12) | (fore << 8);
    uint16_t *location = video_memory + cursor_y * SCREEN_WIDTH + cursor_x;
    *location = c | attribute;
    
    cursor_x++;
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT) {
            screen_scroll_once();
            cursor_y = SCREEN_HEIGHT - 1;
        }
    }
    move_cursor();
}

void vga_putc_color(char c, vga_color_t back, vga_color_t fore)
{
    switch (c) {
        case '\t':
            for (int i = 0; i < TAB_LENGTH; i++) {
                vga_putc_raw(' ', back, fore);
            }
            break;
        case '\n':
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= SCREEN_HEIGHT) {
                screen_scroll_once();
                cursor_y = SCREEN_HEIGHT - 1;
            }
            move_cursor();
            break;
        case '\r':
            cursor_x = 0;
            move_cursor();
            break;
        default:
            vga_putc_raw(c, back, fore);
    }
}

void vga_putc(char c)
{
    vga_putc_color(c, BLACK, WHITE);
}

void vga_puts_color(const char *str, vga_color_t back, vga_color_t fore)
{
    while (*str) {
        vga_putc_color(*str++, back, fore);
    }
}

void vga_puts(const char *str)
{
    vga_puts_color(str, BLACK, WHITE);
}

static int vga_vprintk_color(vga_color_t back, vga_color_t fore, const char *format, va_list args)
{
    char buffer[32];
    const char *p = format;
    int chars_written = 0;
    
    while (*p) {
        if (*p != '%') {
            vga_putc_color(*p, back, fore);
            chars_written++;
            p++;
            continue;
        }
        
        p++; // Skip '%'
        
        // Format specifier
        switch (*p) {
            case 's': {
                const char *str = va_arg(args, const char *);
                if (!str) str = "(null)";
                vga_puts_color(str, back, fore);
                chars_written += k_strlen(str);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                vga_putc_color(c, back, fore);
                chars_written++;
                break;
            }
            case 'd': {
                int num = va_arg(args, int);
                k_int_to_string(num, buffer, sizeof(buffer));
                vga_puts_color(buffer, back, fore);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'u': {
                unsigned int num = va_arg(args, unsigned int);
                k_uint_to_string(num, buffer, sizeof(buffer));
                vga_puts_color(buffer, back, fore);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'x': {
                unsigned int num = va_arg(args, unsigned int);
                k_num_to_hexstr((uint64_t)num, false, buffer, sizeof(buffer));
                vga_puts_color(buffer, back, fore);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'X': {
                unsigned int num = va_arg(args, unsigned int);
                k_num_to_hexstr((uint64_t)num, true, buffer, sizeof(buffer));
                vga_puts_color(buffer, back, fore);
                chars_written += k_strlen(buffer);
                break;
            }
            case '%': {
                vga_putc_color('%', back, fore);
                chars_written++;
                break;
            }
            default: {
                vga_putc_color('%', back, fore);
                vga_putc_color(*p, back, fore);
                chars_written += 2;
                break;
            }
        }
        p++;
    }
    
    return chars_written;
}

void vga_printk_color(vga_color_t back, vga_color_t fore, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vga_vprintk_color(back, fore, format, args);
    va_end(args);
}

void vga_printk(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vga_vprintk_color(BLACK, WHITE, format, args);
    va_end(args);
}

void vga_init(void)
{
    clear_screen();
}