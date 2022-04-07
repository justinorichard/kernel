#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

void _start() {
    // Issue a write system call
    write(1, "Hello world!\n", 13);

    char* test_page = (char*)0x400000000;
    test_page[0] = 'h';
    test_page[1] = 'e';
    test_page[2] = 'l';
    test_page[3] = 'l';
    test_page[4] = 'o';
    test_page[5] = '\n';
    write(1, test_page, 6);

    // Loop forever
    for (;;) {
    }
}
