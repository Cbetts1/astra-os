#ifndef PIT_H
#define PIT_H

/*
 * AstraOS Programmable Interval Timer (8253/8254) Driver
 *
 * Configures channel 0 of the PIT to fire IRQ0 at a fixed rate.
 * The default rate is 100 Hz (10 ms per tick).
 *
 * Reference: https://wiki.osdev.org/Programmable_Interval_Timer
 */

#include <stdint.h>

/* Desired timer frequency in Hz */
#define PIT_FREQUENCY_HZ  100

/* Initialise the PIT and register the IRQ0 handler */
void pit_init(void);

/* Return the number of timer ticks since boot */
uint32_t pit_get_ticks(void);

/* Busy-wait for approximately the given number of milliseconds */
void pit_sleep_ms(uint32_t ms);

#endif /* PIT_H */
