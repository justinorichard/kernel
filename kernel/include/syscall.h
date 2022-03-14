#pragma once

#include "idt.h"
#include "stdint.h"

#define SYS_read 0
#define SYS_write 1

extern int syscall(uint64_t nr, ...);
extern void syscall_entry();

void syscall_init();

int syscall_handler(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5);