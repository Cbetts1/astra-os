#ifndef KERNEL_H
#define KERNEL_H

/*
 * AstraOS Kernel - Public Interface
 */

#include <stdint.h>

/* Multiboot information structure (partial - fields used at boot) */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    /* Additional fields omitted for v0.1 */
} multiboot_info_t;

/* Multiboot magic value placed in EAX by the bootloader */
#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

/*
 * kernel_main - C entry point called from boot.s
 *
 * @magic:  Multiboot magic number (should equal MULTIBOOT_BOOTLOADER_MAGIC)
 * @mbi:    Pointer to the multiboot information structure
 */
void kernel_main(uint32_t magic, const multiboot_info_t *mbi);

/*
 * kernel_panic - Print an error message and halt the system
 *
 * @msg:  Null-terminated message string
 */
void kernel_panic(const char *msg);

#endif /* KERNEL_H */
