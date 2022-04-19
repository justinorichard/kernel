#pragma once

#define DEBUG 0
#define debug(fmt)               \
    do {                         \
        if (DEBUG) kprintf(fmt); \
    } while (0)
#define debugf(fmt, ...)                      \
    do {                                      \
        if (DEBUG) kprintf(fmt, __VA_ARGS__); \
    } while (0)