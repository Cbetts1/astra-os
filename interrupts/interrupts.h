#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/*
 * AstraOS Interrupt Subsystem
 *
 * Stub interface for the Interrupt Descriptor Table (IDT) and
 * Programmable Interrupt Controller (PIC) management.
 *
 * Future implementation will provide:
 *   - IDT setup with 256 gate descriptors
 *   - PIC (8259A) initialisation and IRQ remapping
 *   - Exception handlers (division by zero, page fault, etc.)
 *   - IRQ handlers (timer, keyboard, etc.)
 *   - sti() / cli() wrappers
 */

#include <stdint.h>

/* Initialise the interrupt subsystem (no-op in v0.1 stub) */
void interrupts_init(void);

/* Enable hardware interrupts */
void interrupts_enable(void);

/* Disable hardware interrupts */
void interrupts_disable(void);

#endif /* INTERRUPTS_H */
