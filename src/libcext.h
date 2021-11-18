#pragma once

#define RAND_MAX 2147483647

#include <ultra64.h>
#include <stdarg.h>

void *memcpy(void *dst, const void *src, size_t size);
void *memset(void *str, int c, size_t n);
int memcmp(const void *str1, const void *str2, size_t n);
int strncmp(const unsigned char *str1, const unsigned char *str2, size_t n);
int strcmp(const unsigned char *str1, const unsigned char *str2);
size_t strlen(const char *str);
char *strchr(const char *str, int ch);
int vsnprintf(char *str, size_t n, const char *format, va_list arg);
int snprintf(char *str, size_t n, const char *format, ...);
int vsprintf(char *str, const char *format, va_list arg);
void vprintf(const char *format, va_list arg);
void printf(const char *format, ...);
int abs(int x);
void srand(unsigned int seed);
int rand();