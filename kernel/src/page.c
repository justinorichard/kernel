#include "page.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kstdio.h"
#include "kstring.h"

#define PAGE_SIZE 0x1000

typedef struct page_table_entry {
    bool present : 1;
    bool writable : 1;
    bool user : 1;
    uint16_t unused : 9;
    uint64_t address : 51;
    bool no_execute : 1;
} __attribute__((packed)) pt_entry_t;

typedef struct linear_address {
    uint16_t offset : 12;
    uint16_t table : 9;
    uint16_t directory : 9;
    uint16_t directory_ptr : 9;
    uint16_t pml4 : 9;
    uint16_t _unused : 16;
} __attribute__((packed)) linear_address_t;

typedef struct list_node {
    struct list_node* next;
} list_node_t;

typedef struct free_list {
    list_node_t* head;
} free_list_t;

free_list_t free_list;

uintptr_t read_cr3() {
    uintptr_t value;
    __asm__("mov %%cr3, %0" : "=r"(value));
    return value;
}

void print_table_entry(pt_entry_t* page_entry) {
    // check if entry present
    if (!page_entry->present) {
        kprintf("not present\n");
        return;
    }

    // page belongs to user or kernel
    if (page_entry->user) {
        kprintf("user ");
    } else {
        kprintf("kernel ");
    }

    // if writable
    if (page_entry->writable) {
        kprintf("writable ");
    }

    // if executable
    if (!page_entry->no_execute) {
        kprintf("executable ");
    }

    // next-level's kernel table
    kprintf("-> 0x%x\n", page_entry->address << 12);
}

pt_entry_t* translate_entry(pt_entry_t* page_start, int level, uint16_t index) {
    pt_entry_t* page_entry = page_start + index;
    kprintf("  Level %d (index %d of 0x%x)\n", level, index, page_start);
    kprintf("    ");
    print_table_entry(page_entry);

    // return the next-level's page table
    return (pt_entry_t*)(page_entry->address << 12);
}

void translate(void* address) {
    kprintf("Translating 0x%x\n", address);

    pt_entry_t* page_start = (pt_entry_t*)read_cr3();
    linear_address_t* laddress = &address;

    page_start = translate_entry(page_start, 4, laddress->pml4);
    page_start = translate_entry(page_start, 3, laddress->directory_ptr);
    page_start = translate_entry(page_start, 2, laddress->directory);
    page_start = translate_entry(page_start, 1, laddress->table);

    kprintf("0x%x maps to 0x%x\n\n", address, page_start + laddress->offset);
}

uintptr_t pmem_alloc() {
    if (free_list.head == NULL) {
        return NULL;
    }

    uintptr_t rtr = free_list.head;
    free_list.head = free_list.head->next;

    return rtr;
}

void pmem_free(uintptr_t p) {
    list_node_t* new_head = p;
    new_head->next = free_list.head;
    free_list.head = new_head;
}

void init_alloc(struct stivale2_struct_tag_memmap* memmap, struct stivale2_struct_tag_hhdm* hhdm) {
    // init free list
    free_list.head = NULL;

    for (uint64_t i = 0; i < memmap->entries; i++) {
        struct stivale2_mmap_entry entry = memmap->memmap[i];
        if (entry.type == 1) {  // entry is an usable memory
            for (uint64_t curr = entry.base; curr < entry.base + entry.length; curr += PAGE_SIZE) {
                pmem_free(curr);
            }
        }
    }
}

bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable) {
    // init linear address
    linear_address_t* laddress = &address;
    uint16_t addresses[] = {
        0,
        laddress->table,          // level 1
        laddress->directory,      // level 2
        laddress->directory_ptr,  // level 3
        laddress->pml4,           // level 4
    };

    pt_entry_t* table_entry = root;

    for (int level = 4; level >= 1; level--) {
        pt_entry_t* page_entry = table_entry + addresses[level];

        kprintf("level %d table: 0x%x entry: 0x%x\n", level, table_entry, page_entry);

        // create new page if not present
        if (!page_entry->present) {
            kprintf(" not present, make new page\n");
            // allocate new page
            uintptr_t new_page = pmem_alloc();
            if (new_page == NULL) {  // run out of page
                return false;
            }
            memset(new_page, 0, PAGE_SIZE);

            kprintf(" new page %p shift %p\n", new_page, new_page >> 12);

            // link new page to current table and set permission
            page_entry->present = true;
            page_entry->no_execute = level == 1 ? executable : false;
            page_entry->user = level == 1 ? user : true;
            page_entry->writable = level == 1 ? writable : true;
            page_entry->address = new_page >> 12;
        }

        table_entry = page_entry->address << 12;
    }

    return true;
}