/*
 * AstraOS Linker Script
 *
 * Lays out the kernel ELF sections starting at 1 MiB (0x100000),
 * which is the conventional load address for x86 kernels loaded
 * by Multiboot-compliant bootloaders.
 *
 * kernel_end is exported so the Physical Memory Manager knows where
 * the kernel image ends and free physical memory begins.
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

    /*
     * kernel_end marks the first byte after all kernel sections.
     * The PMM uses this to avoid allocating frames in use by the kernel.
     */
    kernel_end = .;
}
