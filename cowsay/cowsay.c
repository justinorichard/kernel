#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "string.h"

void _start() {
    char input_buffer[512];
    memset(input_buffer, 0, 512);

    // source: https://en.wikipedia.org/wiki/Cowsay
    printf(" ________________________________________\n");
    printf("/ You have Egyptian flu: you're going to \\\n");
    printf("\\ be a mummy.                            /\n");
    printf(" ----------------------------------------\n");
    printf("        \\   ^__^\n");
    printf("         \\  (oo)\\_______\n");
    printf("            (__)\\       )\\/\\\n");
    printf("                ||----w |\n");
    printf("                ||     ||\n");

    exit(0);
}
