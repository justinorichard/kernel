#include <stddef.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (*str != '\0') {
        len += 1;
        str += 1;
    }
    return len;
}

void *memset(void *s, int c, size_t n) {
    char *p = s;
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
}