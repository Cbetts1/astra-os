#ifndef KEYBOARD_H
#define KEYBOARD_H

/*
 * AstraOS PS/2 Keyboard Driver
 *
 * Handles IRQ1 (PS/2 keyboard), translates US QWERTY scancodes to
 * ASCII, and maintains a small circular input buffer.
 *
 * Reference: https://wiki.osdev.org/PS/2_Keyboard
 */

#include <stdint.h>

/* Initialise the keyboard driver and register the IRQ1 handler */
void keyboard_init(void);

/*
 * keyboard_getchar - read one character from the input buffer.
 * Returns 0 if the buffer is empty.
 */
char keyboard_getchar(void);

/*
 * keyboard_poll - return non-zero if at least one character is waiting
 */
int keyboard_poll(void);

#endif /* KEYBOARD_H */
