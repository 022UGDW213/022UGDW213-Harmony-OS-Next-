#ifndef HARMONY_KEYBOARD_H
#define HARMONY_KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
char keyboard_get_char(void); // Blocking poll

#endif
