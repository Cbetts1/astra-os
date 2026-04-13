/*
 * AstraOS PS/2 Keyboard Driver
 *
 * Translates AT make-code scancodes (set 1) to US QWERTY ASCII.
 * Break codes (bit 7 set) and extended codes (0xE0 prefix) are
 * silently ignored in this revision.
 *
 * Reference: https://wiki.osdev.org/PS/2_Keyboard
 */

#include "keyboard.h"
#include "../interrupts/idt.h"
#include "../interrupts/pic.h"

/* PS/2 I/O ports */
#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64

/* Input buffer — power-of-two size for cheap modulo via mask */
#define KB_BUF_SIZE   64
#define KB_BUF_MASK   (KB_BUF_SIZE - 1)

static char     kb_buf[KB_BUF_SIZE];
static uint32_t kb_head = 0;   /* producer (IRQ writes here)  */
static uint32_t kb_tail = 0;   /* consumer (getchar reads)    */

/* ------------------------------------------------------------------ */
/*  US QWERTY scancode → ASCII table (make codes 0x00-0x58)           */
/* ------------------------------------------------------------------ */

static const char scancode_map[89] = {
/* 0x00 */ 0,
/* 0x01 */ 0,       /* Escape      */
/* 0x02 */ '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
/* 0x0C */ '-', '=',
/* 0x0E */ '\b',    /* Backspace   */
/* 0x0F */ '\t',    /* Tab         */
/* 0x10 */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
/* 0x1A */ '[', ']',
/* 0x1C */ '\n',    /* Enter       */
/* 0x1D */ 0,       /* Left Ctrl   */
/* 0x1E */ 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
/* 0x27 */ ';', '\'',
/* 0x29 */ '`',
/* 0x2A */ 0,       /* Left Shift  */
/* 0x2B */ '\\',
/* 0x2C */ 'z', 'x', 'c', 'v', 'b', 'n', 'm',
/* 0x33 */ ',', '.', '/',
/* 0x36 */ 0,       /* Right Shift */
/* 0x37 */ '*',
/* 0x38 */ 0,       /* Left Alt    */
/* 0x39 */ ' ',
};

#define SCANCODE_MAP_LEN  ((int)(sizeof(scancode_map) / sizeof(scancode_map[0])))

/* ------------------------------------------------------------------ */
/*  I/O helper                                                          */
/* ------------------------------------------------------------------ */

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* ------------------------------------------------------------------ */
/*  IRQ1 handler                                                        */
/* ------------------------------------------------------------------ */

static void keyboard_irq1_handler(interrupt_frame_t *frame)
{
    (void)frame;

    uint8_t sc = inb(KB_DATA_PORT);

    /* Ignore break codes (key release) and extended codes */
    if (sc & 0x80) return;
    if (sc == 0xE0) return;

    if (sc < SCANCODE_MAP_LEN) {
        char c = scancode_map[sc];
        if (c) {
            uint32_t next = (kb_head + 1) & KB_BUF_MASK;
            if (next != kb_tail) { /* buffer not full */
                kb_buf[kb_head] = c;
                kb_head = next;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void keyboard_init(void)
{
    irq_register_handler(1, keyboard_irq1_handler);
    pic_clear_mask(1);
}

char keyboard_getchar(void)
{
    if (kb_tail == kb_head) return 0; /* empty */
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) & KB_BUF_MASK;
    return c;
}

int keyboard_poll(void)
{
    return kb_tail != kb_head;
}
