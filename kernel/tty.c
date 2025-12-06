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
#include <stdbool.h>
#include <kernel/tty.h>
#include <kernel/screen.h>
#include <kernel/serial.h>

#define TTY_BUFFER_SIZE 4096

static char tty_buffer[TTY_BUFFER_SIZE];
static size_t tty_head = 0; // next write index  
static size_t tty_tail = 0; // next read index  

// Internal: amount of data currently in buffer  
static inline size_t tty_count(void)
{
    return (tty_head - tty_tail) & (TTY_BUFFER_SIZE - 1);
}

void tty_init(void)
{
    tty_head = 0;
    tty_tail = 0;
    scr_init();
    srl_init();
}

typedef char _tty_check[(TTY_BUFFER_SIZE & (TTY_BUFFER_SIZE - 1)) == 0 ? 1 : -1];

size_t tty_write(int fd, const char *buf, size_t len)
{
    (void)fd;
    size_t written = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)*(buf + i);
        size_t next = (tty_head + 1) & (TTY_BUFFER_SIZE - 1);
        if (next == tty_tail) {
            tty_tail = (tty_tail + 1) & (TTY_BUFFER_SIZE - 1);
        }
        tty_buffer[tty_head] = (char)c;
        tty_head = next;
        written++;
    }

    tty_flush();
    return written;
}

size_t tty_putc(char c)
{
    char ch = c;
    return tty_write(0, &ch, 1);
}

size_t tty_read(char *dest, size_t len)
{
    size_t available = tty_count();
    size_t to_read = (len < available) ? len : available;
    for (size_t i = 0; i < to_read; i++) {
        dest[i] = tty_buffer[tty_tail];
        tty_tail = (tty_tail + 1) & (TTY_BUFFER_SIZE - 1);
    }
    return to_read;
}

void tty_flush(void)
{
    char tmp[256];
    size_t cnt = tty_count();
    while (cnt > 0) {
        size_t to_copy = (cnt < sizeof(tmp)) ? cnt : sizeof(tmp);
        for (size_t i = 0; i < to_copy; i++) {
            tmp[i] = tty_buffer[tty_tail];
            tty_tail = (tty_tail + 1) & (TTY_BUFFER_SIZE - 1);
        }
        scr_write(tmp, to_copy);
        srl_write(tmp, to_copy);

        cnt = tty_count();
    }
}
