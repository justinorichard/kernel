#include "string.h"

#include <stddef.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (*str != '\0') {
        len += 1;
        str += 1;
    }
    return len;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

void *memset(void *s, int c, size_t n) {
    char *p = s;
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *cdest = (char *)dest;
    char *csrc = (char *)src;
    for (size_t i = 0; i < n; i++) {
        cdest[i] = csrc[i];
    }
}