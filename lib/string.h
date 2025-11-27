#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void memcpy(void *dest, const void *src, size_t len);
void memset(void *dest, uint8_t val, size_t len);
void bzero(void *dest, size_t len);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
size_t strlen(const char *src);
char *uint_to_string(uint32_t num, char *buffer, size_t buffer_size);
char *int_to_string(int32_t num, char *buffer, size_t buffer_size);
char *num_to_hexstr(uint32_t number, bool need_0x, char *buffer, size_t buffer_size);
char *strreverse(char *str);

#endif
