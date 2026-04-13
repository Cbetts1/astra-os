/*
 * AstraOS Memory Manager - Stub Implementation
 *
 * This file is a placeholder for the full memory management subsystem.
 *
 * Planned implementation phases:
 *   Phase 1 - Physical Memory Manager
 *     - Parse the Multiboot memory map to find usable RAM regions.
 *     - Maintain a bitmap of free/used physical page frames (4 KiB each).
 *     - Expose pmm_alloc_frame() / pmm_free_frame().
 *
 *   Phase 2 - Paging / VMM
 *     - Build initial page directory and page tables.
 *     - Identity-map the kernel and VGA buffer.
 *     - Enable CR0.PG to activate paging.
 *
 *   Phase 3 - Kernel Heap
 *     - Implement kmalloc() / kfree() using a slab or buddy allocator
 *       backed by the VMM.
 */

#include "memory.h"

void memory_init(void)
{
    /*
     * TODO: Parse multiboot memory map passed from kernel_main.
     * TODO: Initialise physical memory bitmap.
     * TODO: Set up page directory and enable paging.
     */
}

void *kmalloc(size_t size)
{
    /* TODO: Implement kernel heap allocator */
    (void)size;
    return (void *)0;
}

void kfree(void *ptr)
{
    /* TODO: Implement kernel heap free */
    (void)ptr;
}
