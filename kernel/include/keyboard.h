#pragma once

#include "idt.h"

__attribute__((interrupt)) void keyboard_handler(interrupt_context_t *ctx);

/**
 * @brief kgetc returns a character input from the keyboard.
 * The function blocks until a keyboard input is received.
 *
 * @return char the character input by keyboard
 */
char kgetc();