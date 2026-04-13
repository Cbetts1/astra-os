#ifndef MEMORY_H
#define MEMORY_H

/*
 * AstraOS Memory Manager
 *
 * Stub interface for the physical and virtual memory subsystems.
 *
 * Future implementation will provide:
 *   - Physical Memory Manager (PMM) using a bitmap allocator
 *   - Paging with a page-directory / page-table setup
 *   - Kernel heap (kmalloc / kfree) via a slab or pool allocator
 */

#include <stddef.h>
#include <stdint.h>

/* Initialise the memory manager (no-op in v0.1 stub) */
void memory_init(void);

/*
 * Allocate `size` bytes from the kernel heap.
 * Returns NULL until the heap allocator is implemented.
 */
void *kmalloc(size_t size);

/* Free a previously allocated block */
void kfree(void *ptr);

#endif /* MEMORY_H */
