#ifndef IDT_H
#define IDT_H

/*
 * AstraOS Interrupt Descriptor Table (IDT)
 *
 * The IDT holds 256 gate descriptors that map CPU vectors (0-255) to
 * handler routines.  Vectors 0-31 are CPU exceptions; vectors 32-47
 * (0x20-0x2F) are remapped hardware IRQs; vectors 48+ are available
 * for software interrupts.
 *
 * Each gate descriptor is 8 bytes (64 bits):
 *   [15:0]   handler offset bits 15:0
 *   [31:16]  code segment selector (0x08 = kernel CS)
 *   [39:32]  zero
 *   [47:40]  type+attributes (0x8E = 32-bit interrupt gate, ring 0)
 *   [63:48]  handler offset bits 31:16
 */

#include <stdint.h>

/* Interrupt frame pushed on the stack by the ISR stub */
typedef struct {
    uint32_t ds;                               /* saved data segment  */
    uint32_t edi, esi, ebp, esp_dummy,
             ebx, edx, ecx, eax;              /* pusha registers     */
    uint32_t int_no, err_code;                 /* vector + error code */
    uint32_t eip, cs, eflags, useresp, ss;    /* CPU auto-pushed     */
} __attribute__((packed)) interrupt_frame_t;

/* Initialise the IDT and load it with LIDT */
void idt_init(void);

/*
 * isr_handler - called from the assembly ISR common stub for exceptions
 * irq_handler - called from the assembly IRQ common stub for hardware IRQs
 */
void isr_handler(interrupt_frame_t *frame);
void irq_handler(interrupt_frame_t *frame);

/*
 * irq_register_handler - install a C callback for a hardware IRQ
 * @irq:     IRQ line number (0-15)
 * @handler: function called when the IRQ fires (may be NULL to unregister)
 */
void irq_register_handler(uint8_t irq, void (*handler)(interrupt_frame_t *));

#endif /* IDT_H */
