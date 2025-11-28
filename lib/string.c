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
#include <lib/string.h>

void memcpy(void *dest, const void *src, size_t len)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    
    for (size_t i = 0; i < len; i++) {
        d[i] = s[i];
    }
}

void memset(void *dest, uint8_t val, size_t len)
{
    uint8_t *d = (uint8_t *)dest;
    for (size_t i = 0; i < len; i++) {
        d[i] = val;
    }
}

void bzero(void *dest, size_t len)
{
    memset(dest, 0, len);
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

size_t strlen(const char *src)
{
    size_t len = 0;
    while (src[len] != '\0') {
        len++;
    }
    return len;
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *d = dest;
    while (*d != '\0') d++;
    while ((*d++ = *src++) != '\0');
    return dest;
}

char *uint_to_string(uint32_t num, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0 || !buffer) return buffer;

    // Make sure there's enough space to store 4294967295
    if (buffer_size < 12) {
        if (buffer_size > 0) buffer[0] = '\0';
        return buffer;
    }
    
    char *ptr = buffer;
    char *end = buffer + buffer_size - 1;
    
    if (num == 0) {
        if (ptr < end) *ptr++ = '0';
        *ptr = '\0';
        return buffer;
    }
    
    // Convert digits in reverse order
    char temp[32];
    char *temp_ptr = temp;
    
    while (num > 0 && temp_ptr < temp + sizeof(temp) - 1) {
        *temp_ptr++ = '0' + (num % 10);
        num /= 10;
    }
    
    // Copy in correct order
    while (temp_ptr > temp && ptr < end) {
        *ptr++ = *--temp_ptr;
    }
    *ptr = '\0';
    
    return buffer;
}

char *int_to_string(int32_t num, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0) return buffer;
    
    if (num < 0) {
        if (buffer_size > 1) {
            buffer[0] = '-';
            return uint_to_string(-num, buffer + 1, buffer_size - 1);
        }
    }
    return uint_to_string(num, buffer, buffer_size);
}

char *num_to_hexstr(uint32_t number, bool need_0x, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0) return buffer;
    
    const char hex_chars[] = "0123456789ABCDEF";
    char *ptr = buffer;
    char *end = buffer + buffer_size - 1;
    
    if (need_0x) {
        if (ptr < end) *ptr++ = '0';
        if (ptr < end) *ptr++ = 'x';
    }
    
    // Convert each nibble
    for (int i = 28; i >= 0; i -= 4) {
        if (ptr < end) {
            *ptr++ = hex_chars[(number >> i) & 0xF];
        }
    }
    *ptr = '\0';
    
    return buffer;
}

char *strreverse(char *str)
{
    if (!str) return str;
    
    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}