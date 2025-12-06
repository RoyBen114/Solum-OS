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
#include <kernel/vsprintf.h>
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

static int vsnprintf_local(char *out, size_t out_sz, const char *fmt, va_list args)
{
    char *o = out;
    size_t rem = out_sz ? out_sz - 1 : 0; // leave space for NUL
    const char *p = fmt;
    char tmp[64];

    while (*p && rem) {
        if (*p != '%') {
            *o++ = *p++;
            rem--;
            continue;
        }
        p++; // skip % 
        int long_count = 0;
        while (*p == 'l') { long_count++; p++; }

        switch (*p) {
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                size_t l = k_strlen(s);
                if (l > rem) l = rem;
                for (size_t i = 0; i < l; i++) { *o++ = s[i]; }
                rem -= l;
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                if (rem) { *o++ = c; rem--; }
                break;
            }
            case 'd': case 'i': {
                if (long_count >= 2) {
                    k_int64_to_string(va_arg(args, int64_t), tmp, sizeof(tmp));
                } else if (long_count == 1) {
                    k_int64_to_string((int64_t)va_arg(args, long), tmp, sizeof(tmp));
                } else {
                    k_int_to_string(va_arg(args, int), tmp, sizeof(tmp));
                }
                size_t l = k_strlen(tmp);
                if (l > rem) l = rem;
                for (size_t i = 0; i < l; i++) *o++ = tmp[i];
                rem -= l;
                break;
            }
            case 'u': {
                if (long_count >= 2) {
                    k_uint64_to_string(va_arg(args, uint64_t), tmp, sizeof(tmp));
                } else if (long_count == 1) {
                    k_uint64_to_string((uint64_t)va_arg(args, unsigned long), tmp, sizeof(tmp));
                } else {
                    k_uint_to_string(va_arg(args, unsigned int), tmp, sizeof(tmp));
                }
                size_t l = k_strlen(tmp);
                if (l > rem) l = rem;
                for (size_t i = 0; i < l; i++) *o++ = tmp[i];
                rem -= l;
                break;
            }
            case 'x': case 'X': {
                bool upper = (*p == 'X');
                if (long_count >= 2) {
                    k_num_to_hexstr(va_arg(args, uint64_t), false, tmp, sizeof(tmp));
                } else if (long_count == 1) {
                    k_num_to_hexstr((uint64_t)va_arg(args, unsigned long), false, tmp, sizeof(tmp));
                } else {
                    k_num_to_hexstr((uint64_t)va_arg(args, unsigned int), false, tmp, sizeof(tmp));
                }
                // tmp contains uppercase hex by implementation; if lower requested, convert
                if (!upper) {
                    for (size_t i = 0; i < k_strlen(tmp); i++) {
                        char c = tmp[i];
                        if (c >= 'A' && c <= 'F') tmp[i] = c - 'A' + 'a';
                    }
                }
                size_t l = k_strlen(tmp);
                if (l > rem) l = rem;
                for (size_t i = 0; i < l; i++) *o++ = tmp[i];
                rem -= l;
                break;
            }
            case 'p': {
                void *ptr = va_arg(args, void*);
                k_num_to_hexstr((uint64_t)(uintptr_t)ptr, true, tmp, sizeof(tmp));
                size_t l = k_strlen(tmp);
                if (l > rem) l = rem;
                for (size_t i = 0; i < l; i++) *o++ = tmp[i];
                rem -= l;
                break;
            }
            case '%': {
                if (rem) { *o++ = '%'; rem--; }
                break;
            }
            default: {
                if (rem) { *o++ = '%'; rem--; }
                if (rem) { *o++ = *p; rem--; }
                break;
            }
        }
        if (*p) p++;
    }

    if (out_sz) *o = '\0';
    return (int)(out_sz ? (out_sz - 1 - rem) : 0);
}

int printk_with_level(int level, const char *format, va_list args)
{
    // Format into kernel buffer
    static char kbuf[2048];
    int len = vsprintf(kbuf, format, args);

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
