#pragma once

#include <stdint.h>

#define SYS_read 0
#define SYS_write 1
#define SYS_mmap 2

extern int syscall(uint64_t number, ...);
