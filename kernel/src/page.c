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
    bool write_through : 1;
    bool cache_disable : 1;
    bool accessed : 1;
    bool dirty : 1;
    bool page_size : 1;
    uint8_t _unused0 : 4;
    uintptr_t address : 40;
    uint16_t _unused1 : 11;
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

free_list_t free_list;    // lsit of free pages
uint64_t virtual_offset;  // hhdm virtual address offset

uintptr_t add_virtual_offset(uintptr_t ptr) { return ptr + virtual_offset; }

uintptr_t read_cr3() {
    uintptr_t value;
    __asm__("mov %%cr3, %0" : "=r"(value));
    return value;
}

uint64_t read_cr0() {
    uintptr_t value;
    __asm__("mov %%cr0, %0" : "=r"(value));
    return value;
}

void write_cr0(uint64_t value) { __asm__("mov %0, %%cr0" : : "r"(value)); }

void write_cr3(uint64_t value) { __asm__("mov %0, %%cr3" : : "r"(value)); }

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
    page_entry = add_virtual_offset(page_entry);

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
    list_node_t* nxt = ((list_node_t*)add_virtual_offset(free_list.head))->next;
    free_list.head = nxt;

    return rtr;
}

void pmem_free(uintptr_t p) {
    list_node_t* new_head = add_virtual_offset(p);
    new_head->next = free_list.head;
    free_list.head = p;
}

// Unmap everything in the lower half of an address space with level 4 page table at address root
void unmap_lower_half(uintptr_t root) {
    // We can reclaim memory used to hold page tables, but NOT the mapped pages
    pt_entry_t* l4_table = add_virtual_offset(root);
    for (size_t l4_index = 0; l4_index < 256; l4_index++) {
        // Does this entry point to a level 3 table?
        if (l4_table[l4_index].present) {
            // Yes. Mark the entry as not present in the level 4 table
            l4_table[l4_index].present = false;

            // Now loop over the level 3 table
            pt_entry_t* l3_table = add_virtual_offset(l4_table[l4_index].address << 12);
            for (size_t l3_index = 0; l3_index < 512; l3_index++) {
                // Does this entry point to a level 2 table?
                if (l3_table[l3_index].present && !l3_table[l3_index].page_size) {
                    // Yes. Loop over the level 2 table
                    pt_entry_t* l2_table = add_virtual_offset(l3_table[l3_index].address << 12);
                    for (size_t l2_index = 0; l2_index < 512; l2_index++) {
                        // Does this entry point to a level 1 table?
                        if (l2_table[l2_index].present && !l2_table[l2_index].page_size) {
                            // Yes. Free the physical page the holds the level 1 table
                            pmem_free(l2_table[l2_index].address << 12);
                        }
                    }
                    // Free the physical page that held the level 2 table
                    pmem_free(l3_table[l3_index].address << 12);
                }
            }
            // Free the physical page that held the level 3 table
            pmem_free(l4_table[l4_index].address << 12);
        }
    }

    // Reload CR3 to flush any cached address translations
    write_cr3(read_cr3());
}

// reload Translation Lookaside Buffer
void invalidate_tlb(uintptr_t virtual_address) {
    __asm__("invlpg (%0)" ::"r"(virtual_address) : "memory");
}

void init_alloc(struct stivale2_struct_tag_memmap* memmap, struct stivale2_struct_tag_hhdm* hhdm) {
    free_list.head = NULL;  // init free list
    virtual_offset = hhdm->addr;

    // Enable write protection
    uint64_t cr0 = read_cr0();
    cr0 |= 0x10000;
    write_cr0(cr0);

    for (uint64_t i = 0; i < memmap->entries; i++) {
        struct stivale2_mmap_entry entry = memmap->memmap[i];
        if (entry.type == 1) {  // entry is an usable memory
            for (uint64_t curr = entry.base; curr < entry.base + entry.length; curr += PAGE_SIZE) {
                pmem_free(curr);
            }
        }
    }

    unmap_lower_half(read_cr3());
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
        page_entry = add_virtual_offset(page_entry);

        kprintf("level %d table: 0x%x entry: 0x%x\n", level, table_entry, page_entry);

        // create new page if not present
        if (!page_entry->present) {
            kprintf(" not present, make new page\n");

            // allocate new page
            uintptr_t new_page = pmem_alloc();
            if (new_page == NULL) {  // run out of page
                return false;
            }
            memset(add_virtual_offset(new_page), 0, PAGE_SIZE);

            kprintf(" new page %p shift %p\n", new_page, new_page >> 12);

            // link new page to current table and set permission
            page_entry->present = true;
            page_entry->no_execute = level == 1 ? !executable : false;
            page_entry->user = level == 1 ? user : true;
            page_entry->writable = level == 1 ? writable : true;
            page_entry->address = new_page >> 12;
        }

        table_entry = page_entry->address << 12;
    }

    invalidate_tlb(address);

    return true;
}

bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable) {
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
        page_entry = add_virtual_offset(page_entry);

        kprintf("level %d table: 0x%x entry: 0x%x\n", level, table_entry, page_entry);

        // page is not mapped
        if (!page_entry->present) {
            return false;
        }

        page_entry->user |= user;
        page_entry->no_execute &= !executable;
        page_entry->writable |= writable;

        table_entry = page_entry->address << 12;
    }

    invalidate_tlb(address);

    return true;
}