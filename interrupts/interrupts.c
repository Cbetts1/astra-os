/*
 * AstraOS Interrupt Subsystem - Stub Implementation
 *
 * Planned implementation phases:
 *   Phase 1 - IDT
 *     - Define a 256-entry Interrupt Descriptor Table.
 *     - Install default "unhandled interrupt" stubs for all 256 vectors.
 *     - Load the IDT with the LIDT instruction.
 *
 *   Phase 2 - PIC Initialisation
 *     - Send ICW1-ICW4 to the master (0x20) and slave (0xA0) 8259A PICs.
 *     - Remap IRQs 0-15 to vectors 0x20-0x2F (above CPU exceptions).
 *     - Mask all IRQs initially; unmask as drivers register handlers.
 *
 *   Phase 3 - Exception Handlers
 *     - Implement handlers for CPU exceptions (0-31).
 *     - Hook kernel_panic() for unrecoverable faults.
 *
 *   Phase 4 - IRQ Handlers
 *     - Timer (IRQ 0): feed the scheduler tick.
 *     - Keyboard (IRQ 1): deliver scancodes to the input layer.
 */

#include "interrupts.h"

void interrupts_init(void)
{
    /*
     * TODO: Build and load the IDT.
     * TODO: Initialise and remap the 8259A PIC.
     * TODO: Install CPU exception handlers.
     */
}

void interrupts_enable(void)
{
    __asm__ volatile ("sti");
}

void interrupts_disable(void)
{
    __asm__ volatile ("cli");
}
