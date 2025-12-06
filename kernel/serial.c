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

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <kernel/serial.h>
#include <kernel/port.h>
#include <lib/string.h>

#define SERIAL_PORT 0x3F8

static bool serial_is_transmit_empty(void)
{
    return inb(SERIAL_PORT + 5) & 0x20;
}

static bool serial_is_receive_ready(void)
{
    return inb(SERIAL_PORT + 5) & 0x01;
}

void srl_init(void)
{
    outb(SERIAL_PORT + 1, 0x00); // Disable all interrupts
    outb(SERIAL_PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(SERIAL_PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_PORT + 1, 0x00); //                  (hi byte)
    outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_putc(char c)
{
    while (!serial_is_transmit_empty());
    if (c == '\n') {
        while (!serial_is_transmit_empty());
        outb(SERIAL_PORT, '\r');
    }
    outb(SERIAL_PORT, c);
}

void srl_write(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        serial_putc(buf[i]);
    }
}