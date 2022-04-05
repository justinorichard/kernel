#include "kstdio.h"

#include <stdint.h>
#include <string.h>

#include "kstdio.h"
#include "stdarg.h"

term_write_t term_write = NULL;

void set_term_write(term_write_t fn) { term_write = fn; }

// Print a single character to the terminal
void kprint_c(char c) { term_write(&c, 1); }

// Print a string to the terminal
void kprint_s(const char *str) {
    size_t len = strlen(str);
    term_write(str, len);
}

// only support up to Hexadecimal
char radix_digit_map(uint8_t radix) { return radix <= 9 ? '0' + radix : 'a' + (radix - 10); }

// print number respect to its radix
void kprint_r(uint64_t value, uint8_t radix) {
    uint64_t n = 1;

    // corner case
    if (value == 0) {
        kprint_c('0');
        return;
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