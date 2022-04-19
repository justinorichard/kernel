#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "string.h"

void _start() {
    for (;;) {
        // init the input buffer
        char input_buffer[512];
        int next = 0;
        memset(input_buffer, 0, 512);

        // input prompt
        printf("$ ");

        char ch;
        while (read(1, &ch, 1)) {
            input_buffer[next] = ch;
            printf("%c", ch);

            // user done with input
            if (ch == '\n') {
                input_buffer[next] = '\0';
                if (exec(input_buffer, NULL) != 0) {
                    printf("%s not found\n", input_buffer);
                    break;
                }
            }

            next += 1;
        }
    }
}
