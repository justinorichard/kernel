#include <stddef.h>
#include <stdint.h>

#include "stdarg.h"
#include "stivale2.h"
#include "util.h"

// Every interrupt handler must specify a code selector. We'll use entry 5
// (5*8=0x28), which is where our bootloader set up a usable code selector for
// 64-bit mode.
#define IDT_CODE_SELECTOR 0x28

// IDT entry types
#define IDT_TYPE_INTERRUPT 0xE
#define IDT_TYPE_TRAP 0xF

// 64-bit IDT Gate Descriptor
typedef struct idt_entry {
    uint16_t offset_0;
    uint16_t selector;
    uint8_t ist : 3;
    uint8_t _unused_0 : 5;
    uint8_t type : 4;
    uint8_t _unused_1 : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset_1;
    uint32_t offset_2;
    uint32_t _unused_2;
} __attribute__((packed)) idt_entry_t;

idt_entry_t idt[256];

// Reserve space for the stack
static uint8_t stack[8192];

static struct stivale2_tag unmap_null_hdr_tag = {
    .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID, .next = 0};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {.identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
            .next = (uintptr_t)&unmap_null_hdr_tag},
    .flags = 0};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"),
               used)) static struct stivale2_header stivale_hdr = {
    // Use ELF file's default entry point
    .entry_point = 0,

    // Use stack (starting at the top)
    .stack = (uintptr_t)stack + sizeof(stack),

    // Bit 1: request pointers in the higher half
    // Bit 2: enable protected memory ranges (specified in PHDR)
    // Bit 3: virtual kernel mappings (no constraints on physical memory)
    // Bit 4: required
    .flags = 0x1E,

    // First tag struct
    .tags = (uintptr_t)&terminal_hdr_tag};

// Find a tag with a given ID
void *find_tag(struct stivale2_struct *hdr, uint64_t id) {
    // Start at the first tag
    struct stivale2_tag *current = (struct stivale2_tag *)hdr->tags;

    // Loop as long as there are more tags to examine
    while (current != NULL) {
        // Does the current tag match?
        if (current->identifier == id) {
            return current;
        }

        // Move to the next tag
        current = (struct stivale2_tag *)current->next;
    }

    // No matching tag found
    return NULL;
}

typedef void (*term_write_t)(const char *, size_t);
term_write_t term_write = NULL;

void term_setup(struct stivale2_struct *hdr) {
    // Look for a terminal tag
    struct stivale2_struct_tag_terminal *tag =
        find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

    // Make sure we find a terminal tag
    if (tag == NULL) halt();

    // Save the term_write function pointer
    term_write = (term_write_t)tag->term_write;
}

// Print a single character to the terminal
void kprint_c(char c) { term_write(&c, 1); }

size_t strlen(const char *str) {
    size_t len = 0;
    while (*str != '\0') {
        len += 1;
        str += 1;
    }
    return len;
}

// Print a string to the terminal
void kprint_s(const char *str) {
    size_t len = strlen(str);
    term_write(str, len);
}

// only support up to Hexadecimal
char radix_digit_map(uint8_t radix) {
    return radix <= 9 ? '0' + radix : 'a' + (radix - 10);
}

// print number respect to its radix
void kprint_r(uint64_t value, uint8_t radix) {
    uint64_t n = 1;

    // corner case
    if (value == 0) {
        kprint_c('0');
    }

    while ((n * radix <= value) && (n * radix > n)) {
        n *= radix;
    }

    while (n > 0) {
        kprint_c(radix_digit_map(value / n));
        value %= n;
        n /= radix;
    }
}

// Print an unsigned 64-bit integer value to the terminal in decimal notation
// (no leading zeros please!)
void kprint_d(uint64_t value) { kprint_r(value, 10); }

// Print an unsigned 64-bit integer value to the terminal in lowercase
// hexadecimal notation (no leading zeros or “0x” please!)
void kprint_x(uint64_t value) { kprint_r(value, 16); }

// Print the value of a pointer to the terminal in lowercase hexadecimal with
// the prefix “0x”
void kprint_p(void *ptr) {
    kprint_s("0x");
    kprint_x((uint64_t)ptr);
}

void kprintf(const char *format, ...) {
    // Start processing variadic arguments
    va_list args;
    va_start(args, format);

    // Loop until we reach the end of the format string
    size_t index = 0;
    while (format[index] != '\0') {
        // Is the current charater a '%'?
        if (format[index] == '%') {
            // Yes, print the argument
            index++;
            switch (format[index]) {
                case '%':
                    kprint_c('%');
                    break;
                case 'c':
                    kprint_c(va_arg(args, int));
                    break;
                case 's':
                    kprint_s(va_arg(args, char *));
                    break;
                case 'd':
                    kprint_d(va_arg(args, uint64_t));
                    break;
                case 'x':
                    kprint_x(va_arg(args, int64_t));
                    break;
                case 'p':
                    kprint_p(va_arg(args, void *));
                    break;
                default:
                    kprint_s("<not supported>");
            }
        } else {
            // No, just a normal character. Print it.
            kprint_c(format[index]);
        }
        index++;
    }

    // Finish handling variadic arguments
    va_end(args);
}

typedef struct interrupt_context {
    uintptr_t ip;
    uint64_t cs;
    uint64_t flags;
    uintptr_t sp;
    uint64_t ss;
} __attribute__((packed)) interrupt_context_t;

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

typedef struct idt_record {
    uint16_t size;
    void *base;
} __attribute__((packed)) idt_record_t;

void *memset(void *s, int c, size_t n) {
    char *p = s;
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
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

    // Step 3: Install the IDT
    idt_record_t record = {.size = sizeof(idt), .base = idt};
    __asm__("lidt %0" ::"m"(record));
}

void _start(struct stivale2_struct *hdr) {
    // We've booted! Let's start processing tags passed to use from the
    // bootloader
    term_setup(hdr);

    idt_setup();

    // Print a greeting
    term_write("Hello Kernel!\n", 15);

    // Get virutal memory struct
    struct stivale2_struct_tag_hhdm *hhdm =
        find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

    // Get memmap struct
    struct stivale2_struct_tag_memmap *memmap =
        find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    kprintf("Usable Memory:\n");
    for (uint64_t i = 0; i < memmap->entries; i++) {
        struct stivale2_mmap_entry entry = memmap->memmap[i];
        if (entry.type == 1) {  // print only usable memory
            kprintf("  %x-%x mapped at %x-%x\n", entry.base,
                    entry.base + entry.length, entry.base + hhdm->addr,
                    entry.base + entry.length + hhdm->addr);
        }
    }

    int *p = (int *)0x1;
    *p = 123;

    // We're done, just hang...
    halt();
}