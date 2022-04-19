#pragma once

#include <stddef.h>

typedef void (*term_write_t)(const char *, size_t);

// pass the functon for putting character on terminal to kprintf
// must be called before calling kprintf
void set_term_write(term_write_t fn);

// kernel implementation of printf
// support %c %x %d %x %p format character
void kprintf(const char *format, ...);