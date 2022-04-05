#pragma once

#include <stddef.h>
#include <stdint.h>

typedef void (*void_function_t)();

void_function_t load(uintptr_t p, size_t size);