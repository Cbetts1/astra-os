#ifndef KERNEL_H
#define KERNEL_H

/*
 * AstraOS Kernel - Public Interface
 */

#include <stdint.h>

/*
 * Multiboot memory-map entry.
 * The 'size' field does NOT include the 4 bytes of 'size' itself, so
 * the next entry is at (uint8_t *)entry + entry->size + 4.
 */
typedef struct {
    uint32_t size;      /* size of this struct minus the 'size' field  */
    uint64_t addr;      /* base physical address of the region         */
    uint64_t len;       /* length of the region in bytes               */
    uint32_t type;      /* 1 = available RAM, anything else = reserved */
} __attribute__((packed)) mmap_entry_t;

/*
 * Full Multiboot1 information structure.
 * Only the fields we actually use are named; the rest are included for
 * correct offset calculation.
 */
typedef struct {
    uint32_t flags;         /* which fields are valid (bit mask)        */
    uint32_t mem_lower;     /* amount of lower memory in KiB            */
    uint32_t mem_upper;     /* amount of upper memory in KiB            */
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;   /* bytes in the memory map                  */
    uint32_t mmap_addr;     /* physical address of mmap_entry_t array   */
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
} __attribute__((packed)) multiboot_info_t;

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
