#ifndef VGA_H
#define VGA_H

/*
 * AstraOS VGA Text Mode Driver
 *
 * Provides basic character output using the PC VGA text buffer at
 * physical address 0xB8000.  Supports an 80x25 character screen with
 * 16 foreground and 16 background colours.
 */

#include <stddef.h>
#include <stdint.h>

/* Hardware text-mode colour constants */
typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;

/* Initialise the VGA driver and clear the screen */
void vga_init(void);

/* Clear the screen and reset cursor to (0, 0) */
void vga_clear(void);

/* Set the current text colour */
void vga_set_color(vga_color_t fg, vga_color_t bg);

/* Write a single character at the current cursor position */
void vga_putchar(char c);

/* Write a null-terminated string at the current cursor position */
void vga_puts(const char *str);

#endif /* VGA_H */
