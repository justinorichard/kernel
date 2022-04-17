#pragma once

#include <stddef.h>

size_t strlen(const char *str);

int strcmp(const char *str1, const char *str2);

void *memset(void *s, int c, size_t n);

void *memcpy(void *dest, const void *src, size_t n);