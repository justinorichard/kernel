#pragma once

#include <stddef.h>

#include "stivale2.h"

// Halt the CPU in an infinite loop
static void halt() {
    while (1) {
        __asm__("hlt");
    }
}

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