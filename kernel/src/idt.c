#include "idt.h"

#include <string.h>

#include "gdt.h"
#include "keyboard.h"
#include "kstdio.h"
#include "pic.h"
#include "port.h"
#include "util.h"

idt_entry_t idt[256];

__attribute__((interrupt)) void divide_error_handler(interrupt_context_t *ctx) {
    kprintf("divide error handler\n");
    halt();
};

__attribute__((interrupt)) void debug_exception_handler(interrupt_context_t *ctx) {
    kprintf("debug exception handler\n");
    halt();
}

__attribute__((interrupt)) void nonmaskable_interrupt_handler(interrupt_context_t *ctx) {
    kprintf("nonmaskable interrupt handler\n");
    halt();
}

__attribute__((interrupt)) void breakpoint_handler(interrupt_context_t *ctx) {
    kprintf("breakpoint handler\n");
    halt();
};

__attribute__((interrupt)) void overflow_handler(interrupt_context_t *ctx) {
    kprintf("overflow handler\n");
    halt();
};

__attribute__((interrupt)) void bound_range_exceeded_handler(interrupt_context_t *ctx) {
    kprintf("BOUND range exceeded handler\n");
    halt();
};

__attribute__((interrupt)) void invalid_opcode_handler(interrupt_context_t *ctx) {
    kprintf("invalid opcode handler\n");
    halt();
}

__attribute__((interrupt)) void device_not_available_handler(interrupt_context_t *ctx) {
    kprintf("device not available handler\n");
    halt();
}

__attribute__((interrupt)) void double_fault_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("double fault handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void coprocessor_segment_overrun_handler(interrupt_context_t *ctx) {
    kprintf("coprocessor segment overrun fault handler\n");
    halt();
}

__attribute__((interrupt)) void invalid_tss_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("invalid TSS handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void segment_not_present_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("segment not present handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void stack_segment_fault_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("stack segment fault handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void general_protection_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("general protection handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void page_fault_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("page fault handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void x87_fpu_floating_point_handler(interrupt_context_t *ctx) {
    kprintf("x87 FPU floating point handler\n");
    halt();
}

__attribute__((interrupt)) void alignment_check_handler(interrupt_context_t *ctx, uint64_t ec) {
    kprintf("alignment check handler (ec=%d)\n", ec);
    halt();
}

__attribute__((interrupt)) void machine_check_handler(interrupt_context_t *ctx) {
    kprintf("machine check handler\n");
    halt();
}

__attribute__((interrupt)) void simd_floating_point_exception_handler(interrupt_context_t *ctx) {
    kprintf("SIMD floating-point exception handler\n");
    halt();
}

__attribute__((interrupt)) void virtualization_exception_handler(interrupt_context_t *ctx) {
    kprintf("virtualization exception handler\n");
    halt();
}

__attribute__((interrupt)) void control_protection_exception_handler_ec(interrupt_context_t *ctx,
                                                                        uint64_t ec) {
    kprintf("control protection exception handler (ec=%d)\n", ec);
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
    idt[index].dpl = 3;      // run the handler in kernel mode
    idt[index].ist = 0;      // not using an interrupt stack table
    idt[index].selector = KERNEL_CODE_SELECTOR;
}

/**
 * Initialize an interrupt descriptor table, set handlers for standard
 * exceptions, and install the IDT.
 */
void idt_setup() {
    // zero out the IDT
    memset(&idt, 0, sizeof(idt_entry_t) * 256);

    // set handler for the standard exceptions (1-21)
    idt_set_handler(0, divide_error_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(1, debug_exception_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(2, nonmaskable_interrupt_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(3, breakpoint_handler, IDT_TYPE_TRAP);
    idt_set_handler(4, overflow_handler, IDT_TYPE_TRAP);
    idt_set_handler(5, bound_range_exceeded_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(6, invalid_opcode_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(7, device_not_available_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(8, double_fault_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(9, coprocessor_segment_overrun_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(10, invalid_tss_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(11, segment_not_present_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(12, stack_segment_fault_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(13, general_protection_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(14, page_fault_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(16, x87_fpu_floating_point_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(17, alignment_check_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(18, machine_check_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(19, simd_floating_point_exception_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(20, virtualization_exception_handler, IDT_TYPE_INTERRUPT);
    idt_set_handler(21, control_protection_exception_handler_ec, IDT_TYPE_INTERRUPT);

    // set up the handler for pic
    idt_set_handler(IRQ1_INTERRUPT, &keyboard_handler, IDT_TYPE_INTERRUPT);
    pic_unmask_irq(1);

    // Step 3: Install the IDT
    idt_record_t record = {.size = sizeof(idt), .base = idt};
    __asm__("lidt %0" ::"m"(record));
}