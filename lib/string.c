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
    if (len == 0 || dest == src) return;

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    /* Small copies - unrolled byte copy is fastest for tiny sizes */
    if (len < 16) {
        for (size_t i = 0; i < len; i++) d[i] = s[i];
        return;
    }

    /* Align destination to 8 bytes for faster word stores */
    while (((uintptr_t)d & 0x7) && len) {
        *d++ = *s++;
        len--;
    }

    uint64_t *wd = (uint64_t *)d;
    const uint64_t *ws = (const uint64_t *)s;

    size_t words = len / 8;
    for (size_t i = 0; i < words; i++) wd[i] = ws[i];

    d = (uint8_t *)(wd + words);
    s = (const uint8_t *)(ws + words);
    size_t rem = len & 7;
    for (size_t i = 0; i < rem; i++) d[i] = s[i];
}

void k_memset(void *dest, uint8_t val, size_t len)
{
    if (len == 0) return;
    uint8_t *d = (uint8_t *)dest;

    /* Fill leading bytes until aligned */
    while (((uintptr_t)d & 0x7) && len) {
        *d++ = val;
        len--;
    }

    uint64_t pattern = (uint8_t)val;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    uint64_t *wd = (uint64_t *)d;
    size_t words = len / 8;
    for (size_t i = 0; i < words; i++) wd[i] = pattern;

    d = (uint8_t *)(wd + words);
    size_t rem = len & 7;
    for (size_t i = 0; i < rem; i++) d[i] = val;
}

void k_bzero(void *dest, size_t len)
{
    k_memset(dest, 0, len);
}

int k_strcmp(const char *str1, const char *str2)
{
    /* Compare bytes until alignment or difference */
    const uint8_t *a = (const uint8_t *)str1;
    const uint8_t *b = (const uint8_t *)str2;

    while (((uintptr_t)a & 0x7) && ((uintptr_t)b & 0x7)) {
        if (*a != *b) return (int)*a - (int)*b;
        if (*a == 0) return 0;
        a++; b++;
    }

    const uint64_t *wa = (const uint64_t *)a;
    const uint64_t *wb = (const uint64_t *)b;

    for (;;) {
        uint64_t va = *wa;
        uint64_t vb = *wb;
        if (va != vb) {
            /* find differing byte */
            a = (const uint8_t *)wa;
            b = (const uint8_t *)wb;
            for (size_t i = 0; i < 8; i++) {
                if (a[i] != b[i]) return (int)a[i] - (int)b[i];
                if (a[i] == 0) return 0;
            }
        }
        /* check for zero byte quickly */
        if (((va - 0x0101010101010101ULL) & ~va & 0x8080808080808080ULL) != 0) {
            a = (const uint8_t *)wa;
            for (size_t i = 0; i < 8; i++) {
                if (a[i] == 0) return 0;
            }
        }
        wa++; wb++;
    }
}

int k_strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0) return 0;
    while (n-- && *s1 && (*s1 == *s2)) {
        if (n == 0) break;
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *k_strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) d[i] = src[i];
    for (; i < n; i++) d[i] = '\0';
    return dest;
}

int k_memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *a = (const uint8_t *)s1;
    const uint8_t *b = (const uint8_t *)s2;

    /* Small sizes - byte compare */
    if (n < 8) {
        for (size_t i = 0; i < n; i++) {
            if (a[i] != b[i]) return (int)a[i] - (int)b[i];
        }
        return 0;
    }

    /* Align to 8 bytes and compare words */
    while (((uintptr_t)a & 0x7) && n) {
        if (*a != *b) return (int)*a - (int)*b;
        a++; b++; n--;
    }

    const uint64_t *wa = (const uint64_t *)a;
    const uint64_t *wb = (const uint64_t *)b;
    size_t words = n / 8;
    for (size_t i = 0; i < words; i++) {
        if (wa[i] != wb[i]) {
            /* find differing byte within the word */
            a = (const uint8_t *)(wa + i);
            b = (const uint8_t *)(wb + i);
            for (size_t j = 0; j < 8; j++) if (a[j] != b[j]) return (int)a[j] - (int)b[j];
        }
    }
    a = (const uint8_t *)(wa + words);
    b = (const uint8_t *)(wb + words);
    size_t rem = n & 7;
    for (size_t i = 0; i < rem; i++) {
        if (a[i] != b[i]) return (int)a[i] - (int)b[i];
    }
    return 0;
}

void k_memmove(void *dest, const void *src, size_t len)
{
    if (!len || dest == src) return;
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    if (d < s || d >= (s + len)) {
        k_memcpy(d, s, len);
        return;
    }
    /* Overlap, copy backwards */
    d += len;
    s += len;
    while (len--) {
        *--d = *--s;
    }
}

char *k_strchr(const char *s, int c)
{
    char ch = (char)c;
    while (*s) {
        if (*s == ch) return (char *)s;
        s++;
    }
    return (ch == '\0') ? (char *)s : NULL;
}

char *k_strrchr(const char *s, int c)
{
    const char *last = NULL;
    char ch = (char)c;
    while (*s) {
        if (*s == ch) last = s;
        s++;
    }
    if (ch == '\0') return (char *)s;
    return (char *)last;
}

int k_atoi(const char *str)
{
    int sign = 1;
    long result = 0;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    if (*str == '+' || *str == '-') {
        if (*str == '-') sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return (int)(result * sign);
}

size_t k_strlen(const char *src)
{
    const char *s = src;
    /* Handle bytes until aligned */
    while (((uintptr_t)s & 0x7) && *s) s++;

    const uint64_t *w = (const uint64_t *)s;
    for (;;) {
        uint64_t v = *w;
        /* magic to detect zero byte in word */
        if (((v - 0x0101010101010101ULL) & ~v & 0x8080808080808080ULL) != 0) {
            const uint8_t *p = (const uint8_t *)w;
            for (int i = 0; i < 8; i++) {
                if (p[i] == 0) return (size_t)((p + i) - (const uint8_t *)src);
            }
        }
        w++;
    }
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