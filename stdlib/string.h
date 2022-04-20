#pragma once

#include <stddef.h>

/**
 * @brief strlen return the length of the given string
 *
 * @param str
 * @return size_t the length of the string
 */
size_t strlen(const char *str);

/**
 * @brief strcmp compares the given str1 and str2
 * it returns 0 if the str1 equals str2
 * it returns <0 if contents of str1 has a lower value than str2,
 * >0 otherwise
 *
 * @param str1
 * @param str2
 * @return int
 */
int strcmp(const char *str1, const char *str2);

/**
 * @brief memset size the memory begins at address s with size n to c
 *
 * @param s memory address
 * @param c memory size
 * @param n value
 * @return void*
 */
void *memset(void *s, int c, size_t n);

/**
 * @brief memcpy copy memory from address src with size n to dest
 *
 * @param dest destination of copy
 * @param src  source of copy
 * @param n memory size
 * @return void*
 */
void *memcpy(void *dest, const void *src, size_t n);