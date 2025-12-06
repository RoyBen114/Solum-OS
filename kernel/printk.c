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

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/printk.h>
#include <kernel/serial.h>
#include <kernel/screen.h>
#include <kernel/tty.h>
#include <kernel/vsnprintf.h>
#include <lib/string.h>

static const char *level_tag(int level)
{
    switch (level) {
        case 0: return "[EMERG] ";
        case 1: return "[ALERT] ";
        case 2: return "[CRIT] ";
        case 3: return "[ERR] ";
        case 4: return "[WARN] ";
        case 5: return "[NOTICE] ";
        case 6: return "[INFO] ";
        case 7: return "[DEBUG] ";
        default: return "[INFO] ";
    }
}

static vga_color_t level_color(int level)
{
    switch (level) {
        case 0: case 1: case 2: case 3: return LIGHT_RED;
        case 4: return YELLOW;
        case 5: return LIGHT_GREEN;
        case 6: return LIGHT_GREY;
        case 7: return LIGHT_CYAN;
        default: return LIGHT_GREY;
    }
}

int printk_with_level(int level, const char *format, va_list args)
{
    // Format into kernel buffer
    static char kbuf[1024];
    int len = vsnprintf(kbuf, sizeof(kbuf), format, args);

    // Prepend tag by moving buffer content if needed
    const char *tag = level_tag(level);
    char finalbuf[2300];
    k_strcpy(finalbuf, tag);
    k_strcat(finalbuf, kbuf);

    size_t flen = k_strlen(finalbuf);

    vga_color_t color = level_color(level);
    
    tty_write(0, finalbuf, flen, color, BLACK);
    return (int)flen;
}

int printk(const char *format, ...)
{
    if (!format) return 0;

    int level = 6; // default INFO
    const char *p = format;
    if (*p == '<') {
        p++;
        int v = 0;
        bool got = false;
        while (*p >= '0' && *p <= '9') { got = true; v = v * 10 + (*p - '0'); p++; }
        if (got && *p == '>') { level = v; p++; format = p; }
    }

    va_list args;
    va_start(args, format);
    int res = printk_with_level(level, format, args);
    va_end(args);
    return res;
}
