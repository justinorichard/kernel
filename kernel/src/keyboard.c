#include "idt.h"
#include "kstdio.h"
#include "pic.h"
#include "port.h"

__attribute__((interrupt)) void keyboard_handler(interrupt_context_t *ctx) {
    uint8_t scan_code = inb(0x60);  // read a keyboard scan code

    kprintf("%x\n", scan_code);

    outb(PIC1_COMMAND, PIC_EOI);  // end of interrupt message
}