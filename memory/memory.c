/*
 * AstraOS Memory Manager
 *
 * Initialises the Physical Memory Manager from the Multiboot memory map
 * and provides a simple bump-pointer heap allocator for kernel use.
 *
 * The heap region lives in a static array.  This is intentionally simple
 * for v0.2; a slab or buddy allocator will replace it when virtual memory
 * is enabled.
 */

#include "memory.h"
#include "pmm.h"
#include "../kernel/kernel.h"

/* ------------------------------------------------------------------ */
/*  Bump-pointer heap                                                   */
/* ------------------------------------------------------------------ */

#define HEAP_SIZE  (256 * 1024)  /* 256 KiB static kernel heap */

static uint8_t  heap_area[HEAP_SIZE] __attribute__((aligned(16)));
static uint32_t heap_offset = 0;

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void memory_init(const multiboot_info_t *mbi, uint32_t kernel_end_phys)
{
    pmm_init(mbi, kernel_end_phys);
}

void *kmalloc(size_t size)
{
    /* Align to 16 bytes */
    size = (size + 15u) & ~15u;

    if (heap_offset + size > HEAP_SIZE) {
        kernel_panic("kmalloc: kernel heap exhausted");
    }

    void *ptr = (void *)(heap_area + heap_offset);
    heap_offset += (uint32_t)size;
    return ptr;
}

void kfree(void *ptr)
{
    /* Bump allocator does not support individual free */
    (void)ptr;
}
