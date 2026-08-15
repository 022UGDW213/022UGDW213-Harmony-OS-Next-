#ifndef HARMONY_STRING_H
#define HARMONY_STRING_H

#include <stddef.h>

// Minimal freestanding libc replacements (no libc is linked with -nostdlib).
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int   strcmp(const char* a, const char* b);
size_t strlen(const char* s);

#endif // HARMONY_STRING_H
