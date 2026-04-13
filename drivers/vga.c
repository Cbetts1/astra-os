/*
 * AstraOS VGA Text Mode Driver
 *
 * Directly writes character cells to the VGA text buffer at 0xB8000.
 * Each cell is two bytes: the low byte is the ASCII character code and
 * the high byte is the colour attribute (fg nibble | bg nibble << 4).
 */

#include "vga.h"

#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000

static uint16_t * const vga_buffer = (uint16_t *)VGA_MEMORY;
static size_t vga_row  = 0;
static size_t vga_col  = 0;
static uint8_t vga_color;

/* Pack a foreground/background pair into one attribute byte */
static inline uint8_t make_color(vga_color_t fg, vga_color_t bg)
{
    return (uint8_t)fg | ((uint8_t)bg << 4);
}

/* Pack an ASCII character and colour attribute into one VGA cell word */
static inline uint16_t make_entry(unsigned char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* Scroll the screen up by one line */
static void vga_scroll(void)
{
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] =
                vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    /* Clear the last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            make_entry(' ', vga_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_init(void)
{
    vga_color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void)
{
    vga_row = 0;
    vga_col = 0;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = make_entry(' ', vga_color);
        }
    }
}

void vga_set_color(vga_color_t fg, vga_color_t bg)
{
    vga_color = make_color(fg, bg);
}

void vga_putchar(char c)
{
    if (c == '\n') {
        vga_col = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }

    if (c == '\r') {
        vga_col = 0;
        return;
    }

    vga_buffer[vga_row * VGA_WIDTH + vga_col] =
        make_entry((unsigned char)c, vga_color);

    if (++vga_col == VGA_WIDTH) {
        vga_col = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

void vga_puts(const char *str)
{
    while (*str) {
        vga_putchar(*str++);
    }
}
