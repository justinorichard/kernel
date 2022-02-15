#pragma once

#include "idt.h"

__attribute__((interrupt)) void keyboard_handler(interrupt_context_t *ctx);

char kgetc();