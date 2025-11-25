#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "serial.h"
#include "port.h"

int serial_is_transmit_empty() {
    return inb(0x3F8 + 5) & 0x20;
}

void serial_init() {
    outb(0x3F8 + 1, 0x00); 
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00); 
    outb(0x3F8 + 3, 0x03); 
    outb(0x3F8 + 2, 0xC7); 
    outb(0x3F8 + 4, 0x0B); 
}

void serial_putc(char c) {
    while (!serial_is_transmit_empty());
    outb(0x3F8, c);
    
    if (c == '\n') {
        while (!serial_is_transmit_empty());
        outb(0x3F8, '\r');
    }
}

void serial_puts(const char *str) {
    while (*str) {
        serial_putc(*str++);
    }
}

void serial_put_dec(uint64_t value) {
    char buffer[21];
    char *ptr = buffer + 20;
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

void serial_put_hex(uint64_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(buffer);
}

void serial_put_hex_lower(uint64_t value) {
    const char hex_chars[] = "0123456789abcdef";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(buffer);
}

void serial_put_ptr(void *ptr) {
    serial_put_hex((uint64_t)(uintptr_t)ptr);
}

void serial_put_dec_signed(int64_t value) {
    if (value < 0) {
        serial_putc('-');
        value = -value;
    }
    serial_put_dec((uint64_t)value);
}

void serial_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            int long_count = 0;
            while (*format == 'l') {
                long_count++;
                format++;
            }
            
            switch (*format) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    serial_putc(c);
                    break;
                }
                
                case 's': {
                    char *str = va_arg(args, char*);
                    if (str == NULL) {
                        serial_puts("(null)");
                    } else {
                        serial_puts(str);
                    }
                    break;
                }
                
                case 'd': 
                case 'i': {
                    if (long_count >= 2) {
                        int64_t num = va_arg(args, int64_t);
                        serial_put_dec_signed(num);
                    } else if (long_count == 1) {
                        long num = va_arg(args, long);
                        serial_put_dec_signed(num);
                    } else {
                        int num = va_arg(args, int);
                        serial_put_dec_signed(num);
                    }
                    break;
                }
                
                case 'u': {
                    if (long_count >= 2) {
                        uint64_t num = va_arg(args, uint64_t);
                        serial_put_dec(num);
                    } else if (long_count == 1) {
                        unsigned long num = va_arg(args, unsigned long);
                        serial_put_dec(num);
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        serial_put_dec(num);
                    }
                    break;
                }
                
                case 'x': {
                    if (long_count >= 2) {
                        uint64_t num = va_arg(args, uint64_t);
                        serial_put_hex_lower(num);
                    } else if (long_count == 1) {
                        unsigned long num = va_arg(args, unsigned long);
                        serial_put_hex_lower(num);
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        serial_put_hex_lower((uint64_t)num);
                    }
                    break;
                }
                
                case 'X': {
                    if (long_count >= 2) {
                        uint64_t num = va_arg(args, uint64_t);
                        serial_put_hex(num);
                    } else if (long_count == 1) {
                        unsigned long num = va_arg(args, unsigned long);
                        serial_put_hex(num);
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        serial_put_hex((uint64_t)num);
                    }
                    break;
                }
                
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    serial_put_ptr(ptr);
                    break;
                }
                
                case '%': {
                    serial_putc('%');
                    break;
                }
                
                default: {
                    serial_putc('%');
                    serial_putc(*format);
                    break;
                }
            }
        } else {
            serial_putc(*format);
        }
        format++;
    }
    
    va_end(args);
}