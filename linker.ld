/*
 * AstraOS Linker Script
 *
 * Lays out the kernel ELF sections starting at 1 MiB (0x100000),
 * which is the conventional load address for x86 kernels loaded
 * by Multiboot-compliant bootloaders.
 */

ENTRY(_start)

SECTIONS {
    /* Load the kernel at the 1 MiB mark in physical memory */
    . = 1M;

    /* Executable code - multiboot header must appear first */
    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }

    /* Read-only data (string literals, const arrays, etc.) */
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    /* Initialized read-write data */
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    /* Uninitialized data (zero-filled by loader) */
    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }
}
