#pragma once

#include "idt.h"
#include "stdint.h"
#include "stivale2.h"

#define SYS_read 0
#define SYS_write 1
#define SYS_mmap 2
#define SYS_exec 3
#define SYS_exit 4

extern int syscall(uint64_t nr, ...);
extern void syscall_entry();

void syscall_init(struct stivale2_struct_tag_modules *modules);

int syscall_handler(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5);