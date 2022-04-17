#pragma once

#include <stddef.h>

typedef long ssize_t;

ssize_t write(int fd, const void *buf, size_t count);

ssize_t read(int fd, void *buf, size_t count);

int exec(const char *file_name, char *const argv[]);

int exit(int status);
