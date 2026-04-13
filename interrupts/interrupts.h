#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/*
 * AstraOS Interrupt Subsystem
 *
 * Sets up the full interrupt infrastructure:
 *   - 8259A PIC remapped to vectors 0x20-0x2F
 *   - 256-entry Interrupt Descriptor Table (IDT)
 *   - Assembly stubs for CPU exceptions 0-31
 *   - Assembly stubs for hardware IRQs 0-15 (vectors 32-47)
 *   - Per-IRQ handler registration via irq_register_handler()
 */

#include <stdint.h>
#include "idt.h"

/* Initialise PIC, IDT, and load the IDT register */
void interrupts_init(void);

/* Enable hardware interrupts (STI) */
void interrupts_enable(void);

/* Disable hardware interrupts (CLI) */
void interrupts_disable(void);

/*
 * irq_register_handler - install a C callback for a hardware IRQ
 * @irq:     IRQ line number (0-15)
 * @handler: function to call when this IRQ fires
 */
void irq_register_handler(uint8_t irq, void (*handler)(interrupt_frame_t *));

#endif /* INTERRUPTS_H */
