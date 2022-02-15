#include "idt.h"
#include "kstdio.h"
#include "pic.h"
#include "port.h"

#define BUFFER_SIZE 32

// keyboard scan code look up table
int scancode_table[] = {
    // first line
    0, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 8, 9,
    // second line
    113, 119, 101, 114, 116, 121, 117, 105, 111, 112, 91, 93, 10, 0,
    // third line
    97, 115, 100, 102, 103, 104, 106, 107, 108, 59, 39, 96,
    // fourth line
    0, 92, 122, 120, 99, 118, 98, 110, 109, 44, 46, 47, 0};

// circular buffer
int buffer[BUFFER_SIZE];
int buffer_start = 0;
int buffer_end = 0;
int buffer_count = 0;

__attribute__((interrupt)) void keyboard_handler(interrupt_context_t *ctx) {
    uint8_t scancode = inb(0x60);  // read a keyboard scan code

    // only handle press event for now
    if (scancode <= 0x58) {
        // add to buffer when it is not full
        if (buffer_count < BUFFER_SIZE) {
            buffer[buffer_end] = scancode_table[scancode];
            buffer_end = (buffer_end + 1) % BUFFER_SIZE;
            buffer_count += 1;
        }
    }
    kprintf("input received!\n");

    outb(PIC1_COMMAND, PIC_EOI);  // end of interrupt message
}

char kgetc() {
    // wait for new input in the buffer
    while (buffer_count == 0) {
        kprintf("wait for buffer\n");
    }

    kprintf("escape from loop\n");
    // move buffer start to next pos
    char rtr = buffer[buffer_start];
    buffer_start = (buffer_start + 1) % BUFFER_SIZE;
    buffer_count -= 1;

    return rtr;
}