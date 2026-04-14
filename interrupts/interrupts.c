/*
 * AstraOS Interrupt Subsystem
 *
 * Orchestrates the full interrupt initialisation sequence:
 *   1. Remap the 8259A PIC so IRQ0-15 land on vectors 32-47
 *   2. Build and load the 256-entry IDT
 *   3. Enable hardware interrupts (STI)
 */

#include "interrupts.h"
#include "pic.h"
#include "idt.h"

void interrupts_init(void)
{
    pic_init();
    idt_init();
    pic_clear_mask(2);  /* unmask cascade line so slave IRQs 8-15 can fire */
}

void interrupts_enable(void)
{
    __asm__ volatile ("sti");
}

void interrupts_disable(void)
{
    __asm__ volatile ("cli");
}
