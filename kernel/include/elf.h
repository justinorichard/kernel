#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stivale2.h"

typedef void (*void_function_t)();

void_function_t load(uintptr_t p, size_t size);

void exec_module(struct stivale2_module module);