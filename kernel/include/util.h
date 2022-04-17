#pragma once

#include <stddef.h>

#include "stivale2.h"

// Halt the CPU in an infinite loop
static void halt() {
    while (1) {
        __asm__("hlt");
    }
}