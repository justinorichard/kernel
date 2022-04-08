#include "stdlib.h"

#include "mman.h"
#include "stdio.h"

#define PAGE_SIZE 0x1000

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

void* bump = NULL;
size_t space_remaining = 0;

void* malloc(size_t size) {
    // Round sz up to a multiple of 16
    size = ROUND_UP(size, 16);

    // Do we have enoug2h space to satisfy this allocation?
    if (space_remaining < size) {
        // No. Get some more space using `mmap`
        size_t rounded_up = ROUND_UP(size, PAGE_SIZE);
        void* newmem =
            mmap(NULL, rounded_up, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

        // Check for errors
        if (newmem == NULL) {
            return NULL;
        }

        bump = newmem;
        space_remaining = rounded_up;
    }

    // Grab bytes from the beginning of our bump pointer region
    void* result = bump;
    bump += size;
    space_remaining -= size;

    return result;
}

void free(void* p) {
    // Do nothing
}