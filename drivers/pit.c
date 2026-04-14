/*
 * AstraOS PIT Driver
 *
 * Sets PIT channel 0 to square-wave mode at PIT_FREQUENCY_HZ and
 * installs an IRQ0 handler that increments the global tick counter.
 *
 * Reference: https://wiki.osdev.org/Programmable_Interval_Timer
 */

#include "pit.h"
#include "../interrupts/idt.h"
#include "../interrupts/pic.h"

/* PIT I/O ports */
#define PIT_CHANNEL0  0x40
#define PIT_CMD       0x43

/*
 * PIT base frequency is 1193180 Hz (derived from the original IBM PC
 * crystal oscillator at 14.31818 MHz divided by 12).
 */
#define PIT_BASE_HZ   1193180UL

/* Counter divisor for the desired frequency */
#define PIT_DIVISOR   ((uint16_t)(PIT_BASE_HZ / PIT_FREQUENCY_HZ))

/*
 * Command byte:
 *   [7:6] = 00  channel 0
 *   [5:4] = 11  access mode: lo/hi byte
 *   [3:1] = 011 mode 3 (square wave generator)
 *   [0]   = 0   binary (not BCD)
 */
#define PIT_CMD_BYTE  0x36

static volatile uint32_t ticks = 0;

/* ------------------------------------------------------------------ */
/*  I/O helpers                                                         */
/* ------------------------------------------------------------------ */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ------------------------------------------------------------------ */
/*  IRQ0 handler                                                        */
/* ------------------------------------------------------------------ */

static void pit_irq0_handler(interrupt_frame_t *frame)
{
    (void)frame;
    ticks++;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void pit_init(void)
{
    /* Send command byte: channel 0, lo/hi, square wave */
    outb(PIT_CMD, PIT_CMD_BYTE);

    /* Send divisor low byte then high byte */
    outb(PIT_CHANNEL0, (uint8_t)(PIT_DIVISOR & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((PIT_DIVISOR >> 8) & 0xFF));

    /* Register IRQ0 handler and unmask the timer line */
    irq_register_handler(0, pit_irq0_handler);
    pic_clear_mask(0);
}

uint32_t pit_get_ticks(void)
{
    return ticks;
}

void pit_sleep_ms(uint32_t ms)
{
    /* Each tick is 1000/PIT_FREQUENCY_HZ milliseconds */
    uint32_t ticks_needed = (ms * PIT_FREQUENCY_HZ + 999) / 1000;
    uint32_t end = ticks + ticks_needed;
    while ((int32_t)(end - ticks) > 0) {
        __asm__ volatile ("hlt");
    }
}
