/*
 * AstraOS Physical Memory Manager
 *
 * Bitmap-based allocator: one bit per 4 KiB page frame.
 *   0 = free, 1 = used / reserved
 *
 * On init we:
 *  1. Mark every frame as used (conservative start)
 *  2. Walk the Multiboot memory map and mark "type 1" regions as free
 *  3. Re-mark the first 1 MiB (BDA/BIOS/VGA) and the kernel image as used
 *
 * Reference: https://wiki.osdev.org/Page_Frame_Allocation
 */

#include "pmm.h"
#include "../kernel/kprintf.h"

/* ------------------------------------------------------------------ */
/*  Bitmap storage — 128 KiB covers 4 GiB at 4 KiB/frame             */
/* ------------------------------------------------------------------ */

#define BITMAP_WORDS  (PMM_MAX_FRAMES / 32)

static uint32_t bitmap[BITMAP_WORDS];  /* placed in BSS → zero-filled */
static uint32_t total_frames = 0;
static uint32_t free_frames  = 0;

/* ------------------------------------------------------------------ */
/*  Bit manipulation helpers                                            */
/* ------------------------------------------------------------------ */

static inline void bitmap_set(uint32_t frame)
{
    bitmap[frame / 32] |= (1u << (frame % 32));
}

static inline void bitmap_clear(uint32_t frame)
{
    bitmap[frame / 32] &= ~(1u << (frame % 32));
}

static inline int bitmap_test(uint32_t frame)
{
    return (bitmap[frame / 32] >> (frame % 32)) & 1u;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void pmm_init(const multiboot_info_t *mbi, uint32_t kernel_end_phys)
{
    /* Step 1: mark everything as used */
    for (uint32_t i = 0; i < BITMAP_WORDS; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    /* Step 2: walk the Multiboot memory map */
    if (!(mbi->flags & (1u << 6))) {
        /* No memory map provided — fall back to mem_upper */
        uint32_t upper_kb = mbi->mem_upper;
        uint32_t upper_bytes = upper_kb * 1024;
        uint32_t start_frame = (1u << 20) / PMM_FRAME_SIZE; /* 1 MiB */
        uint32_t end_frame   = (0x100000u + upper_bytes) / PMM_FRAME_SIZE;

        for (uint32_t f = start_frame; f < end_frame && f < PMM_MAX_FRAMES; f++) {
            bitmap_clear(f);
            free_frames++;
            total_frames++;
        }
    } else {
        uint32_t addr = mbi->mmap_addr;
        uint32_t end  = mbi->mmap_addr + mbi->mmap_length;

        while (addr < end) {
            const mmap_entry_t *entry = (const mmap_entry_t *)addr;

            if (entry->type == 1) { /* Available RAM */
                uint64_t region_start = entry->addr;
                uint64_t region_len   = entry->len;

                /* Skip regions beyond 4 GiB (we're 32-bit) */
                if (region_start >= 0x100000000ULL) {
                    addr += entry->size + 4;
                    continue;
                }
                if (region_start + region_len > 0x100000000ULL) {
                    region_len = 0x100000000ULL - region_start;
                }

                uint32_t first_frame = (uint32_t)(region_start / PMM_FRAME_SIZE);
                uint32_t num_frames  = (uint32_t)(region_len   / PMM_FRAME_SIZE);

                for (uint32_t f = first_frame;
                     f < first_frame + num_frames && f < PMM_MAX_FRAMES;
                     f++) {
                    bitmap_clear(f);
                    free_frames++;
                    total_frames++;
                }
            }
            addr += entry->size + 4;
        }
    }

    /* Step 3: re-mark unusable regions as used */

    /* First 1 MiB: BIOS data area, VGA, BIOS ROM */
    uint32_t low_frames = (1u << 20) / PMM_FRAME_SIZE; /* 256 frames */
    for (uint32_t f = 0; f < low_frames && f < PMM_MAX_FRAMES; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            free_frames--;
        }
    }

    /* Kernel image: from physical address 0x100000 to kernel_end_phys */
    uint32_t k_start = (1u << 20) / PMM_FRAME_SIZE;
    uint32_t k_end   = (kernel_end_phys + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;
    for (uint32_t f = k_start; f < k_end && f < PMM_MAX_FRAMES; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            free_frames--;
        }
    }

    kprintf("  PMM: %u MB total, %u MB free (%u frames)\n",
            (total_frames * PMM_FRAME_SIZE) >> 20,
            (free_frames  * PMM_FRAME_SIZE) >> 20,
            free_frames);
}

uint32_t pmm_alloc_frame(void)
{
    for (uint32_t i = 0; i < BITMAP_WORDS; i++) {
        if (bitmap[i] == 0xFFFFFFFF) continue; /* all used, skip */
        for (int bit = 0; bit < 32; bit++) {
            uint32_t frame = i * 32 + (uint32_t)bit;
            if (!bitmap_test(frame)) {
                bitmap_set(frame);
                free_frames--;
                return frame * PMM_FRAME_SIZE;
            }
        }
    }
    return 0; /* Out of memory */
}

void pmm_free_frame(uint32_t phys_addr)
{
    uint32_t frame = phys_addr / PMM_FRAME_SIZE;
    if (frame < PMM_MAX_FRAMES && bitmap_test(frame)) {
        bitmap_clear(frame);
        free_frames++;
    }
}

uint32_t pmm_free_frames(void)
{
    return free_frames;
}

uint32_t pmm_total_frames(void)
{
    return total_frames;
}
