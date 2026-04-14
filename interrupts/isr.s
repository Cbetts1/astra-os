/*
 * AstraOS ISR / IRQ Assembly Stubs
 *
 * Each interrupt needs a tiny assembly trampoline that:
 *  1. Optionally pushes a dummy error code (for exceptions that don't have one)
 *  2. Pushes the vector number
 *  3. Saves all general-purpose registers (pusha) and the data segment
 *  4. Loads the kernel data segment selectors
 *  5. Calls the C handler (isr_handler or irq_handler) with a pointer to
 *     the interrupt frame on the stack
 *  6. Restores everything and executes iret
 *
 * The resulting layout on the stack when the C handler is called matches
 * the interrupt_frame_t struct defined in idt.h.
 *
 * Segment selectors (set by GRUB's GDT):
 *   0x08 = flat 32-bit kernel code segment (ring 0)
 *   0x10 = flat 32-bit kernel data segment (ring 0)
 */

/* ------------------------------------------------------------------ */
/*  Macros to generate stubs                                            */
/* ------------------------------------------------------------------ */

/* Exception WITHOUT a CPU-pushed error code */
.macro ISR_NOERR num
.global isr\num
isr\num:
    push $0          /* dummy error code */
    push $\num       /* vector number    */
    jmp  isr_common_stub
.endm

/* Exception WITH a CPU-pushed error code (CPU already pushed it) */
.macro ISR_ERR num
.global isr\num
isr\num:
    push $\num       /* vector number (error code already on stack) */
    jmp  isr_common_stub
.endm

/* Hardware IRQ stub (no error code) */
.macro IRQ num, vector
.global irq\num
irq\num:
    push $0          /* dummy error code */
    push $\vector    /* vector number    */
    jmp  irq_common_stub
.endm

/* ------------------------------------------------------------------ */
/*  CPU exception stubs (vectors 0-31)                                 */
/*                                                                      */
/*  Exceptions with error codes: 8, 10, 11, 12, 13, 14, 17, 21        */
/*  Everything else: no error code                                      */
/* ------------------------------------------------------------------ */

ISR_NOERR  0   /* Division By Zero                  */
ISR_NOERR  1   /* Debug                             */
ISR_NOERR  2   /* Non-Maskable Interrupt            */
ISR_NOERR  3   /* Breakpoint                        */
ISR_NOERR  4   /* Overflow                          */
ISR_NOERR  5   /* Bound Range Exceeded              */
ISR_NOERR  6   /* Invalid Opcode                    */
ISR_NOERR  7   /* Device Not Available              */
ISR_ERR    8   /* Double Fault                      */
ISR_NOERR  9   /* Coprocessor Segment Overrun       */
ISR_ERR   10   /* Invalid TSS                       */
ISR_ERR   11   /* Segment Not Present               */
ISR_ERR   12   /* Stack-Segment Fault               */
ISR_ERR   13   /* General Protection Fault          */
ISR_ERR   14   /* Page Fault                        */
ISR_NOERR 15   /* Reserved                          */
ISR_NOERR 16   /* x87 Floating-Point                */
ISR_ERR   17   /* Alignment Check                   */
ISR_NOERR 18   /* Machine Check                     */
ISR_NOERR 19   /* SIMD Floating-Point               */
ISR_NOERR 20   /* Virtualisation                    */
ISR_ERR   21   /* Control Protection                */
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

/* ------------------------------------------------------------------ */
/*  Hardware IRQ stubs (vectors 32-47)                                  */
/* ------------------------------------------------------------------ */

IRQ  0, 32    /* PIT timer          */
IRQ  1, 33    /* PS/2 keyboard      */
IRQ  2, 34    /* cascade (slave)    */
IRQ  3, 35    /* COM2               */
IRQ  4, 36    /* COM1               */
IRQ  5, 37    /* LPT2 / sound       */
IRQ  6, 38    /* floppy             */
IRQ  7, 39    /* LPT1 / spurious    */
IRQ  8, 40    /* RTC                */
IRQ  9, 41    /* free / ACPI        */
IRQ 10, 42    /* free               */
IRQ 11, 43    /* free               */
IRQ 12, 44    /* PS/2 mouse         */
IRQ 13, 45    /* FPU                */
IRQ 14, 46    /* primary ATA        */
IRQ 15, 47    /* secondary ATA      */

/* ------------------------------------------------------------------ */
/*  Common exception stub                                               */
/* ------------------------------------------------------------------ */

isr_common_stub:
    pusha                      /* save edi,esi,ebp,esp,ebx,edx,ecx,eax */

    /* Save and reload data segment */
    push %ds
    mov  $0x10, %ax
    mov  %ax,   %ds
    mov  %ax,   %es
    mov  %ax,   %fs
    mov  %ax,   %gs

    push %esp                  /* pass pointer to interrupt_frame_t     */
    call isr_handler
    add  $4, %esp              /* discard frame pointer argument        */

    pop  %ds                   /* restore data segment                  */
    mov  %ds,   %ax            /* reload %ax from the restored %ds      */
    mov  %ax,   %es
    mov  %ax,   %fs
    mov  %ax,   %gs
    popa                       /* restore general-purpose registers     */
    add  $8, %esp              /* discard int_no and err_code           */
    iret

/* ------------------------------------------------------------------ */
/*  Common IRQ stub                                                     */
/* ------------------------------------------------------------------ */

irq_common_stub:
    pusha

    push %ds
    mov  $0x10, %ax
    mov  %ax,   %ds
    mov  %ax,   %es
    mov  %ax,   %fs
    mov  %ax,   %gs

    push %esp
    call irq_handler
    add  $4, %esp

    pop  %ds
    mov  %ds,   %ax            /* reload %ax from the restored %ds      */
    mov  %ax,   %es
    mov  %ax,   %fs
    mov  %ax,   %gs
    popa
    add  $8, %esp
    iret

/* No executable stack */
.section .note.GNU-stack, "", @progbits
