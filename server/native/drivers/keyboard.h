#ifndef HARMONY_KEYBOARD_H
#define HARMONY_KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
char keyboard_get_char(void);           // Blocking poll
int  keyboard_get_char_nb(char* c);     // Non-blocking: 1 if a key read

#endif
