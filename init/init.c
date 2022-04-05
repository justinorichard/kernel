#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

void _start() {
    // Issue a write system call
    write(1, "Hello world!\n", 13);

    // Loop forever
    for (;;) {
    }
}
