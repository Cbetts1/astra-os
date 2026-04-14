/*
 * AstraOS IDT Implementation
 *
 * Installs 256 interrupt gate descriptors and loads the IDT register.
 * CPU exceptions (0-31) call isr_handler(); hardware IRQs (32-47) call
 * irq_handler().  Both are defined below and dispatch to registered
 * per-vector callbacks.
 *
 * Reference: Intel SDM Vol. 3A §6.10, §6.14
 */

#include "idt.h"
#include "pic.h"
#include "../kernel/kprintf.h"
#include "../kernel/kernel.h"

/* ------------------------------------------------------------------ */
/*  IDT structures                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t base_low;   /* handler address [15:0]  */
    uint16_t sel;        /* segment selector        */
    uint8_t  zero;       /* always 0                */
    uint8_t  flags;      /* type + DPL + present    */
    uint16_t base_high;  /* handler address [31:16] */
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;      /* size of IDT - 1         */
    uint32_t base;       /* linear address of IDT   */
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

/* ------------------------------------------------------------------ */
/*  External ISR/IRQ stubs (defined in isr.s)                         */
/* ------------------------------------------------------------------ */

extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

/* ------------------------------------------------------------------ */
/*  IRQ handler registration table                                     */
/* ------------------------------------------------------------------ */

#define NUM_IRQS 16

typedef void (*irq_handler_t)(interrupt_frame_t *);
static irq_handler_t irq_handlers[NUM_IRQS];

void irq_register_handler(uint8_t irq, irq_handler_t handler)
{
    if (irq < NUM_IRQS) {
        irq_handlers[irq] = handler;
    }
}

/* ------------------------------------------------------------------ */
/*  Helpers                                                             */
/* ------------------------------------------------------------------ */

/* 32-bit interrupt gate, ring-0, present */
#define IDT_GATE_FLAGS  0x8E

static void idt_set_gate(uint8_t vector, uint32_t base)
{
    idt[vector].base_low  = (uint16_t)(base & 0xFFFF);
    idt[vector].sel       = 0x08;          /* kernel code segment    */
    idt[vector].zero      = 0;
    idt[vector].flags     = IDT_GATE_FLAGS;
    idt[vector].base_high = (uint16_t)((base >> 16) & 0xFFFF);
}

/* ------------------------------------------------------------------ */
/*  Exception names                                                     */
/* ------------------------------------------------------------------ */

static const char *exception_names[] = {
    "Division By Zero",          "Debug",
    "Non-Maskable Interrupt",    "Breakpoint",
    "Overflow",                  "Bound Range Exceeded",
    "Invalid Opcode",            "Device Not Available",
    "Double Fault",              "Coprocessor Segment Overrun",
    "Invalid TSS",               "Segment Not Present",
    "Stack-Segment Fault",       "General Protection Fault",
    "Page Fault",                "Reserved",
    "x87 Floating-Point",        "Alignment Check",
    "Machine Check",             "SIMD Floating-Point",
    "Virtualisation",            "Control Protection",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved",
    "Hypervisor Injection",      "VMM Communication",
    "Security Exception",        "Reserved"
};

/* ------------------------------------------------------------------ */
/*  Interrupt / IRQ dispatch                                           */
/* ------------------------------------------------------------------ */

void isr_handler(interrupt_frame_t *frame)
{
    uint32_t vec = frame->int_no;
    const char *name = (vec < 32) ? exception_names[vec] : "Unknown";

    kprintf("\n[EXCEPTION #%u: %s]  err=0x%x  eip=0x%x  cs=0x%x\n",
            vec, name, frame->err_code, frame->eip, frame->cs);

    kernel_panic("Unhandled CPU exception");
}

void irq_handler(interrupt_frame_t *frame)
{
    uint8_t irq = (uint8_t)(frame->int_no - PIC_MASTER_OFFSET);

    /*
     * Detect and discard spurious IRQs.
     *
     * A spurious IRQ7 occurs when the master PIC raises a phantom interrupt
     * (e.g. due to a noise glitch).  The ISR bit 7 will NOT be set, so we
     * must skip the handler and, critically, skip the EOI to master.
     *
     * A spurious IRQ15 occurs the same way on the slave.  The master *did*
     * receive a real cascade signal on IR2, so we must acknowledge the master
     * (using IRQ2) but skip the handler and the slave EOI.
     */
    if (irq == 7) {
        uint16_t isr = pic_read_isr();
        if (!(isr & (1u << 7))) {
            return; /* spurious IRQ7 — no EOI to master */
        }
    } else if (irq == 15) {
        uint16_t isr = pic_read_isr();
        if (!(isr & (1u << 15))) {
            pic_send_eoi(2); /* spurious IRQ15 — EOI to master only (cascade) */
            return;
        }
    }

    if (irq < NUM_IRQS && irq_handlers[irq]) {
        irq_handlers[irq](frame);
    }

    pic_send_eoi(irq);
}

/* ------------------------------------------------------------------ */
/*  IDT initialisation                                                  */
/* ------------------------------------------------------------------ */

void idt_init(void)
{
    /* CPU exceptions 0-31 */
    idt_set_gate(0,  (uint32_t)isr0);
    idt_set_gate(1,  (uint32_t)isr1);
    idt_set_gate(2,  (uint32_t)isr2);
    idt_set_gate(3,  (uint32_t)isr3);
    idt_set_gate(4,  (uint32_t)isr4);
    idt_set_gate(5,  (uint32_t)isr5);
    idt_set_gate(6,  (uint32_t)isr6);
    idt_set_gate(7,  (uint32_t)isr7);
    idt_set_gate(8,  (uint32_t)isr8);
    idt_set_gate(9,  (uint32_t)isr9);
    idt_set_gate(10, (uint32_t)isr10);
    idt_set_gate(11, (uint32_t)isr11);
    idt_set_gate(12, (uint32_t)isr12);
    idt_set_gate(13, (uint32_t)isr13);
    idt_set_gate(14, (uint32_t)isr14);
    idt_set_gate(15, (uint32_t)isr15);
    idt_set_gate(16, (uint32_t)isr16);
    idt_set_gate(17, (uint32_t)isr17);
    idt_set_gate(18, (uint32_t)isr18);
    idt_set_gate(19, (uint32_t)isr19);
    idt_set_gate(20, (uint32_t)isr20);
    idt_set_gate(21, (uint32_t)isr21);
    idt_set_gate(22, (uint32_t)isr22);
    idt_set_gate(23, (uint32_t)isr23);
    idt_set_gate(24, (uint32_t)isr24);
    idt_set_gate(25, (uint32_t)isr25);
    idt_set_gate(26, (uint32_t)isr26);
    idt_set_gate(27, (uint32_t)isr27);
    idt_set_gate(28, (uint32_t)isr28);
    idt_set_gate(29, (uint32_t)isr29);
    idt_set_gate(30, (uint32_t)isr30);
    idt_set_gate(31, (uint32_t)isr31);

    /* Remapped hardware IRQs 0-15 → vectors 32-47 */
    idt_set_gate(32, (uint32_t)irq0);
    idt_set_gate(33, (uint32_t)irq1);
    idt_set_gate(34, (uint32_t)irq2);
    idt_set_gate(35, (uint32_t)irq3);
    idt_set_gate(36, (uint32_t)irq4);
    idt_set_gate(37, (uint32_t)irq5);
    idt_set_gate(38, (uint32_t)irq6);
    idt_set_gate(39, (uint32_t)irq7);
    idt_set_gate(40, (uint32_t)irq8);
    idt_set_gate(41, (uint32_t)irq9);
    idt_set_gate(42, (uint32_t)irq10);
    idt_set_gate(43, (uint32_t)irq11);
    idt_set_gate(44, (uint32_t)irq12);
    idt_set_gate(45, (uint32_t)irq13);
    idt_set_gate(46, (uint32_t)irq14);
    idt_set_gate(47, (uint32_t)irq15);

    /* Load IDT register */
    idt_ptr.limit = (uint16_t)(sizeof(idt) - 1);
    idt_ptr.base  = (uint32_t)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}
