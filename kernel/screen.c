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
#include <kernel/screen.h>
#include <lib/string.h>
#include <kernel/port.h>

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

void clear_screen(void)
{
    cursor_x = 0;
    cursor_y = 0;
    
    const uint16_t blank = ' ' | (0 << 8);
    for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_memory[i] = blank;
    }
    move_cursor();
}

void screen_scroll_once(void)
{
    for (size_t j = 0; j < SCREEN_HEIGHT - 1; j++) {
        for (size_t i = 0; i < SCREEN_WIDTH; i++) {
            video_memory[j * SCREEN_WIDTH + i] = video_memory[(j + 1) * SCREEN_WIDTH + i];
        }
    }
    
    const uint16_t blank = ' ' | (0 << 8);
    for (size_t i = 0; i < SCREEN_WIDTH; i++) {
        video_memory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i] = blank;
    }
    
    if (cursor_y > 0) {
        cursor_y--;
    }
    move_cursor();
}

static void vga_putc_raw(char c, int back, int fore)
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

void vga_putc(char c, vga_color_t fore, vga_color_t back)
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

void scr_write(const char *buf, size_t len, vga_color_t fore, vga_color_t back)
{
    for (size_t i = 0; i < len; i++) {
        vga_putc(buf[i], fore, back);
    }
}

void scr_init(void)
{
    clear_screen();
}