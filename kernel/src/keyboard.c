#include "idt.h"
#include "kstdio.h"
#include "pic.h"
#include "port.h"
#include "stdbool.h"

#define BUFFER_SIZE 32

// keyboard scan code look up table
int scancode_table[] = {
    // first line
    NULL, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9,
    // second line
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10, NULL,
    // third line
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    // fourth line
    NULL, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', NULL};

int upper_scancode_table[] = {
    // first line
    NULL, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8, 9,
    // second line
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10, NULL,
    // third line
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    // fourth line
    NULL, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', NULL};

// circular buffer
volatile int buffer[BUFFER_SIZE];
volatile int buffer_start = 0;
volatile int buffer_end = 0;
volatile int buffer_count = 0;

// modifiers status
volatile bool lshift_pressed = false;
volatile bool rshift_pressed = false;
volatile bool capslock_pressed = false;

__attribute__((interrupt)) void keyboard_handler(interrupt_context_t *ctx) {
    uint8_t scancode = inb(0x60);  // read a keyboard scan code

    switch (scancode) {
        case 0x2A:  // left shift pressed
            lshift_pressed = true;
            break;
        case 0x36:  // right shift pressed
            rshift_pressed = true;
            break;
        case 0xAA:  // left shift released
            lshift_pressed = false;
            break;
        case 0xB6:  // right shift released
            rshift_pressed = false;
            break;
        case 0x3A:  // capslock pressed
            capslock_pressed = !capslock_pressed;
            break;
        default:
            // ignore key release event
            if (scancode > 0x58) {
                break;
            }
            // add to buffer when it is not full
            if (buffer_count < BUFFER_SIZE) {
                buffer[buffer_end] = (lshift_pressed || rshift_pressed || capslock_pressed)
                                         ? upper_scancode_table[scancode]
                                         : scancode_table[scancode];
                buffer_end = (buffer_end + 1) % BUFFER_SIZE;
                buffer_count += 1;
            }
    }

    outb(PIC1_COMMAND, PIC_EOI);  // end of interrupt message
}

char kgetc() {
    // wait for new input in the buffer
    while (buffer_count == 0) {
    }

    // move buffer start to next pos
    char rtr = buffer[buffer_start];
    buffer_start = (buffer_start + 1) % BUFFER_SIZE;
    buffer_count -= 1;

    return rtr;
}