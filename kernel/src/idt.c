#include "idt.h"

#include "keyboard.h"
#include "kstdio.h"
#include "kstring.h"
#include "pic.h"
#include "port.h"
#include "util.h"

idt_entry_t idt[256];

__attribute__((interrupt)) void example_handler(interrupt_context_t *ctx) {
    kprintf("example interrupt handler\n");
    halt();
}

__attribute__((interrupt)) void example_handler_ec(interrupt_context_t *ctx,
                                                   uint64_t ec) {
    kprintf("example interrupt handler (ec=%d)\n", ec);
    halt();
}

void idt_set_handler(uint8_t index, void *fn, uint8_t type) {
    // set offset for handler
    idt[index].offset_0 = (uint16_t)fn;
    uint64_t offset_1_mask = ((1 << 16) - 1) << 16;
    idt[index].offset_1 = ((uint64_t)fn & offset_1_mask) >> 16;
    idt[index].offset_2 = (uint64_t)fn >> 32;

    idt[index].type = type;
    idt[index].present = 1;  // the entry is present
    idt[index].dpl = 0;      // run the handler in kernel mode
    idt[index].ist = 0;      // not using an interrupt stack table
    idt[index].selector = IDT_CODE_SELECTOR;
}

/**
 * Initialize an interrupt descriptor table, set handlers for standard
 * exceptions, and install the IDT.
 */
void idt_setup() {
    // zero out the IDT
    memset(&idt, 0, sizeof(idt_entry_t) * 256);

    // set handler for the standard exceptions (1-21)
    for (uint8_t i = 1; i <= 21; i++) {
        idt_set_handler(i, &example_handler_ec, IDT_TYPE_INTERRUPT);
    }

    // set up the handler for pic
    idt_set_handler(IRQ1_INTERRUPT, &keyboard_handler, IDT_TYPE_INTERRUPT);
    pic_unmask_irq(1);

    // Step 3: Install the IDT
    idt_record_t record = {.size = sizeof(idt), .base = idt};
    __asm__("lidt %0" ::"m"(record));
}