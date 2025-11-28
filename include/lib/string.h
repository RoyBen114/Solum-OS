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

#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void k_memcpy(void *dest, const void *src, size_t len);
void k_memset(void *dest, uint8_t val, size_t len);
void k_bzero(void *dest, size_t len);
int k_strcmp(const char *str1, const char *str2);
char *k_strcpy(char *dest, const char *src);
char *k_strcat(char *dest, const char *src);
size_t k_strlen(const char *src);
char *k_uint_to_string(uint32_t num, char *buffer, size_t buffer_size);
char *k_int_to_string(int32_t num, char *buffer, size_t buffer_size);
char *k_uint64_to_string(uint64_t num, char *buffer, size_t buffer_size);
char *k_int64_to_string(int64_t num, char *buffer, size_t buffer_size);
char *k_num_to_hexstr(uint64_t number, bool need_0x, char *buffer, size_t buffer_size);
char *k_strreverse(char *str);

#endif
