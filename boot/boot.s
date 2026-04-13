/*
 * AstraOS - Boot Entry Point
 *
 * Multiboot1 header and kernel entry point.
 * This file is the first code executed when the bootloader hands
 * control to the AstraOS kernel.
 */

/* Multiboot1 constants */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* multiboot flag field */
.set MAGIC,    0x1BADB002       /* magic number the bootloader searches for */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum: magic + flags + checksum must equal 0 */

/*
 * Multiboot header - must be in the first 8 KiB of the kernel binary,
 * 32-bit aligned. The bootloader scans for this signature.
 */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
 * The kernel stack lives in BSS (uninitialized data).
 * 16 KiB is adequate for early kernel initialization.
 */
.section .bss
.align 16
stack_bottom:
.skip 16384     /* 16 KiB stack */
stack_top:

/*
 * _start - kernel entry point
 *
 * The bootloader enters here in 32-bit protected mode with:
 *   EAX = 0x2BADB002  (multiboot magic)
 *   EBX = physical address of multiboot info structure
 * Interrupts are disabled; paging is disabled.
 */
.section .text
.global _start
.type _start, @function
_start:
    /* Set up the initial kernel stack */
    mov $stack_top, %esp

    /* Pass multiboot info to kernel_main in cdecl right-to-left order:
     *   kernel_main(uint32_t magic, const multiboot_info_t *mbi)
     *   → push mbi (EBX, 2nd arg) first, then magic (EAX, 1st arg) */
    push %ebx       /* multiboot info pointer (2nd argument) */
    push %eax       /* multiboot magic       (1st argument) */

    /* Transfer control to the C kernel */
    call kernel_main

    /* kernel_main should never return; if it does, halt */
    cli
.hang:
    hlt
    jmp .hang

.size _start, . - _start

/* Tell the linker this object does not need an executable stack */
.section .note.GNU-stack, "", @progbits
