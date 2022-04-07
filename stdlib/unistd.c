#include "unistd.h"

#include <stdint.h>

#include "syscall.h"

ssize_t write(int fd, const void *buf, size_t count) { syscall(SYS_write, fd, buf, count); }

ssize_t read(int fd, void *buf, size_t count) { syscall(SYS_read, fd, buf, count); }