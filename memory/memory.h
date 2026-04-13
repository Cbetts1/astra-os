#ifndef MEMORY_H
#define MEMORY_H

/*
 * AstraOS Memory Manager
 *
 * Provides:
 *   - Physical Memory Manager (PMM) — bitmap allocator, 4 KiB frames
 *   - Kernel bump-heap (kmalloc / kfree) backed by the PMM
 *
 * memory_init() must be called with the Multiboot info pointer and the
 * physical end address of the kernel image so the PMM can correctly
 * protect the kernel's frames from reuse.
 */

#include <stddef.h>
#include <stdint.h>
#include "../kernel/kernel.h"

/*
 * Initialise the PMM and kernel heap.
 * @mbi:             Multiboot information structure
 * @kernel_end_phys: physical address of the first byte after the kernel
 */
void memory_init(const multiboot_info_t *mbi, uint32_t kernel_end_phys);

/*
 * Allocate `size` bytes from the kernel heap (bump allocator).
 * Never returns NULL — calls kernel_panic() on out-of-memory.
 */
void *kmalloc(size_t size);

/* Free a heap block (no-op in bump allocator; kept for API compatibility) */
void kfree(void *ptr);

#endif /* MEMORY_H */
