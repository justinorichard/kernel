#include "syscall.h"

#include "keyboard.h"
#include "kstdio.h"

typedef long ssize_t;  // signed size_t

void syscall_init() { idt_set_handler(0x80, syscall_entry, IDT_TYPE_TRAP); }

ssize_t sys_read(int fd, void *buf, size_t count) {
    size_t index = 0;
    char *buffer = (char *)buf;

    while (index < count) {
        char ch = kgetc();

        if (ch == 8) {  // backspace
            index = index == 0 ? 0 : index - 1;
        } else {
            buffer[index] = ch;
            index += 1;
        }
    }

    return index;
}

ssize_t sys_write(int fd, const void *buf, size_t count) {
    char *buffer = (char *)buf;
    size_t write_count = 0;

    for (size_t i = 0; i < count; i++) {
        kprintf("%c", buffer[i]);
        write_count += 1;
    }

    return write_count;
}

int syscall_handler(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5) {
    switch (nr) {
        case SYS_read:
            return sys_read(arg0, arg1, arg2);
        case SYS_write:
            return sys_write(arg0, arg1, arg2);
        default:
            return -1;
    }
}