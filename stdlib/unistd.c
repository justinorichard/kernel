#include "unistd.h"

#include <stdint.h>

#define SYS_read 0
#define SYS_write 1

extern int syscall(uint64_t number, ...);

ssize_t write(int fd, const void *buf, size_t count) { syscall(SYS_write, fd, buf, count); }

ssize_t read(int fd, void *buf, size_t count) { syscall(SYS_read, fd, buf, count); }