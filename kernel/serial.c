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

void serial_init(void)
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
    /* Send CR before LF to produce CRLF sequence on terminals expecting it */
    if (c == '\n') {
        while (!serial_is_transmit_empty());
        outb(SERIAL_PORT, '\r');
    }
    outb(SERIAL_PORT, c);
}

void serial_puts(const char *str)
{
    while (*str) {
        serial_putc(*str++);
    }
}

static void serial_put_dec(uint64_t value)
{
    char buffer[21];
    char *ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';
    
    if (value == 0) {
        serial_putc('0');
        return;
    }
    
    while (value > 0) {
        *--ptr = '0' + (value % 10);
        value /= 10;
    }
    
    serial_puts(ptr);
}

static void serial_put_hex(uint64_t value, bool uppercase)
{
    const char hex_chars_upper[] = "0123456789ABCDEF";
    const char hex_chars_lower[] = "0123456789abcdef";
    const char *hex_chars = uppercase ? hex_chars_upper : hex_chars_lower;
    
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(buffer);
}

static int serial_vprintk(const char *format, va_list args)
{
    char buffer[32];
    const char *p = format;
    int chars_written = 0;
    
    while (*p) {
        if (*p != '%') {
            serial_putc(*p);
            chars_written++;
            p++;
            continue;
        }
        
        p++; // Skip '%'
        
        // Handle length modifiers
        int long_count = 0;
        while (*p == 'l') {
            long_count++;
            p++;
        }
        
        // Format specifier
        switch (*p) {
            case 's': {
                const char *str = va_arg(args, const char *);
                if (!str) str = "(null)";
                serial_puts(str);
                chars_written += k_strlen(str);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                serial_putc(c);
                chars_written++;
                break;
            }
            case 'd':
            case 'i': {
                if (long_count >= 2) {
                    int64_t num = va_arg(args, int64_t);
                    k_int64_to_string(num, buffer, sizeof(buffer));
                } else if (long_count == 1) {
                    long num = va_arg(args, long);
                    k_int64_to_string((int64_t)num, buffer, sizeof(buffer));
                } else {
                    int num = va_arg(args, int);
                    k_int_to_string(num, buffer, sizeof(buffer));
                }
                serial_puts(buffer);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'u': {
                if (long_count >= 2) {
                    uint64_t num = va_arg(args, uint64_t);
                    k_uint64_to_string(num, buffer, sizeof(buffer));
                } else if (long_count == 1) {
                    unsigned long num = va_arg(args, unsigned long);
                    k_uint64_to_string((uint64_t)num, buffer, sizeof(buffer));
                } else {
                    unsigned int num = va_arg(args, unsigned int);
                    k_uint_to_string(num, buffer, sizeof(buffer));
                }
                serial_puts(buffer);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'x': {
                if (long_count >= 2) {
                    uint64_t num = va_arg(args, uint64_t);
                    k_num_to_hexstr(num, false, buffer, sizeof(buffer));
                } else if (long_count == 1) {
                    unsigned long num = va_arg(args, unsigned long);
                    k_num_to_hexstr((uint64_t)num, false, buffer, sizeof(buffer));
                } else {
                    unsigned int num = va_arg(args, unsigned int);
                    k_num_to_hexstr((uint64_t)num, false, buffer, sizeof(buffer));
                }
                serial_puts(buffer);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'X': {
                if (long_count >= 2) {
                    uint64_t num = va_arg(args, uint64_t);
                    k_num_to_hexstr(num, true, buffer, sizeof(buffer));
                } else if (long_count == 1) {
                    unsigned long num = va_arg(args, unsigned long);
                    k_num_to_hexstr((uint64_t)num, true, buffer, sizeof(buffer));
                } else {
                    unsigned int num = va_arg(args, unsigned int);
                    k_num_to_hexstr((uint64_t)num, true, buffer, sizeof(buffer));
                }
                serial_puts(buffer);
                chars_written += k_strlen(buffer);
                break;
            }
            case 'p': {
                void *ptr = va_arg(args, void*);
                k_num_to_hexstr((uint64_t)(uintptr_t)ptr, true, buffer, sizeof(buffer));
                serial_puts(buffer);
                chars_written += k_strlen(buffer);
                break;
            }
            case '%': {
                serial_putc('%');
                chars_written++;
                break;
            }
            default: {
                serial_putc('%');
                serial_putc(*p);
                chars_written += 2;
                break;
            }
        }
        p++;
    }
    
    return chars_written;
}

void serial_printk(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    serial_vprintk(format, args);
    va_end(args);
}

char serial_read_char(void)
{
    while (!serial_is_receive_ready());
    return inb(SERIAL_PORT);
}

bool serial_read_line(char *buffer, size_t buffer_size)
{
    if (buffer_size == 0) return false;
    
    size_t index = 0;
    char c;
    
    while (index < buffer_size - 1) {
        c = serial_read_char();
        
        // Handle backspace
        if (c == '\b' || c == 0x7F) {
            if (index > 0) {
                index--;
                serial_puts("\b \b"); // Erase character on terminal
            }
            continue;
        }
        
        // Echo character (except newline)
        if (c != '\n' && c != '\r') {
            serial_putc(c);
        }
        
        // Handle end of line
        if (c == '\n' || c == '\r') {
            break;
        }
        
        buffer[index++] = c;
    }
    
    buffer[index] = '\0';
    serial_putc('\n'); // Move to next line
    return true;
}

int serial_scank(const char *format, ...)
{
    if (!format || !*format) {
        return 0;
    }

    char input_buffer[128];
    if (!serial_read_line(input_buffer, sizeof(input_buffer))) {
        return 0;
    }
    
    va_list args;
    va_start(args, format);
    int items_matched = 0;
    const char *f = format;
    char *input = input_buffer;
    
    while (*f && *input) {
        // Skip whitespace in input
        while (*input == ' ' || *input == '\t') input++;
        if (!*input) break;
        
        if (*f != '%') {
            if (*f != *input) break;
            f++;
            input++;
            continue;
        }
        
        f++; // Skip '%'
        
        switch (*f) {
            case 's': {
                char *str = va_arg(args, char*);
                char *start = input;
                
                // Read until whitespace
                while (*input && *input != ' ' && *input != '\t') {
                    *str++ = *input++;
                }
                *str = '\0';
                items_matched++;
                break;
            }
            case 'd': {
                int *num = va_arg(args, int*);
                int value = 0;
                int sign = 1;
                
                if (*input == '-') {
                    sign = -1;
                    input++;
                }
                
                while (*input >= '0' && *input <= '9') {
                    value = value * 10 + (*input - '0');
                    input++;
                }
                
                *num = value * sign;
                items_matched++;
                break;
            }
            case 'u': {
                unsigned int *num = va_arg(args, unsigned int*);
                unsigned int value = 0;
                
                while (*input >= '0' && *input <= '9') {
                    value = value * 10 + (*input - '0');
                    input++;
                }
                
                *num = value;
                items_matched++;
                break;
            }
            case 'x': {
                unsigned int *num = va_arg(args, unsigned int*);
                unsigned int value = 0;
                
                // Skip "0x" if present
                if (input[0] == '0' && (input[1] == 'x' || input[1] == 'X')) {
                    input += 2;
                }
                
                while (1) {
                    if (*input >= '0' && *input <= '9') {
                        value = value * 16 + (*input - '0');
                    } else if (*input >= 'a' && *input <= 'f') {
                        value = value * 16 + (*input - 'a' + 10);
                    } else if (*input >= 'A' && *input <= 'F') {
                        value = value * 16 + (*input - 'A' + 10);
                    } else {
                        break;
                    }
                    input++;
                }
                
                *num = value;
                items_matched++;
                break;
            }
        }
        f++;
    }
    
    va_end(args);
    return items_matched;
}