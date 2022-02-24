#pragma once

#include <stdint.h>

#include "stdbool.h"
#include "stivale2.h"

uintptr_t read_cr3();

/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(void* address);

void init_alloc(struct stivale2_struct_tag_memmap* mmemap, struct stivale2_struct_tag_hhdm* hhdm);

/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on
 * error.
 */
uintptr_t pmem_alloc();

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be
 * page-aligned.
 */
void pmem_free(uintptr_t p);

/**
 * Map a single page of memory into a virtual address space.
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to map into the address space, must be
 * page-aligned
 * \param user Should the page be user-accessible?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if the mapping succeeded, or false if there was an error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);