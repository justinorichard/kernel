#pragma once

#include "stddef.h"

// malloc return a memory address with at least size
// amount of memory assigned to it
void *malloc(size_t size);

void free(void *ptr);