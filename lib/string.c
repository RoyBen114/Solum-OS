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

void k_memcpy(void *dest, const void *src, size_t len)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    
    for (size_t i = 0; i < len; i++) {
        d[i] = s[i];
    }
}

void k_memset(void *dest, uint8_t val, size_t len)
{
    uint8_t *d = (uint8_t *)dest;
    for (size_t i = 0; i < len; i++) {
        d[i] = val;
    }
}

void k_bzero(void *dest, size_t len)
{
    k_memset(dest, 0, len);
}

int k_strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

size_t k_strlen(const char *src)
{
    size_t len = 0;
    while (src[len] != '\0') {
        len++;
    }
    return len;
}

char *k_strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

char *k_strcat(char *dest, const char *src)
{
    char *d = dest;
    while (*d != '\0') d++;
    while ((*d++ = *src++) != '\0');
    return dest;
}

char *k_uint_to_string(uint32_t num, char *buffer, size_t buffer_size)
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

char *k_int_to_string(int32_t num, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0 || !buffer) return buffer;
    
    /* Handle INT32_MIN safely by delegating to 64-bit implementation */
    if (num == (int32_t)0x80000000) {
        return k_int64_to_string((int64_t)num, buffer, buffer_size);
    }
    if (num < 0) {
        if (buffer_size > 1) {
            buffer[0] = '-';
            return k_uint_to_string((uint32_t)(-num), buffer + 1, buffer_size - 1);
        }
        if (buffer_size > 0) buffer[0] = '\0';
        return buffer;
    }
    return k_uint_to_string((uint32_t)num, buffer, buffer_size);
}

char *k_uint64_to_string(uint64_t num, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0 || !buffer) return buffer;

    /* Maximum uint64 is 18446744073709551615 (20 digits) */
    if (buffer_size < 21) {
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

    char temp[32];
    char *temp_ptr = temp;
    while (num > 0 && temp_ptr < temp + sizeof(temp) - 1) {
        *temp_ptr++ = '0' + (num % 10);
        num /= 10;
    }
    while (temp_ptr > temp && ptr < end) {
        *ptr++ = *--temp_ptr;
    }
    *ptr = '\0';
    return buffer;
}

char *k_int64_to_string(int64_t num, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0 || !buffer) return buffer;
    if (num < 0) {
        if (buffer_size > 1) {
            buffer[0] = '-';
            return k_uint64_to_string((uint64_t)(- (uint64_t)num), buffer + 1, buffer_size - 1);
        }
        if (buffer_size > 0) buffer[0] = '\0';
        return buffer;
    }
    return k_uint64_to_string((uint64_t)num, buffer, buffer_size);
}

char *k_num_to_hexstr(uint64_t number, bool need_0x, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0 || !buffer) return buffer;

    const char hex_chars[] = "0123456789ABCDEF";
    char *ptr = buffer;
    char *end = buffer + buffer_size - 1;

    if (need_0x) {
        if (ptr < end) *ptr++ = '0';
        if (ptr < end) *ptr++ = 'x';
    }

    /* Print without leading zeros */
    bool started = false;
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t nib = (number >> i) & 0xF;
        if (nib || started || i == 0) {
            if (ptr < end) *ptr++ = hex_chars[nib];
            started = true;
        }
    }
    *ptr = '\0';
    return buffer;
}

char *k_strreverse(char *str)
{
    if (!str) return str;
    
    size_t len = k_strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}