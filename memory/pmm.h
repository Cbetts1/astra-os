#ifndef PMM_H
#define PMM_H

/*
 * AstraOS Physical Memory Manager
 *
 * Manages physical RAM using a bitmap of 4 KiB page frames.
 * One bit per frame: 0 = free, 1 = used.
 *
 * Usage:
 *   pmm_init(mbi) — called once with the Multiboot info pointer
 *   pmm_alloc_frame() — returns a free frame's physical address, or 0
 *   pmm_free_frame(addr) — marks a frame as free
 */

#include <stdint.h>
#include "../kernel/kernel.h"

#define PMM_FRAME_SIZE  4096      /* 4 KiB frames                    */
#define PMM_MAX_FRAMES  (1u << 20) /* supports up to 4 GiB of RAM   */

/*
 * Initialise the PMM from the Multiboot memory map.
 * Marks all usable regions as free, then marks the kernel and
 * lower-memory regions as used.
 */
void pmm_init(const multiboot_info_t *mbi, uint32_t kernel_end_phys);

/* Allocate one 4 KiB frame; returns physical address, or 0 on failure */
uint32_t pmm_alloc_frame(void);

/* Free a previously allocated frame */
void pmm_free_frame(uint32_t phys_addr);

/* Return the number of free frames */
uint32_t pmm_free_frames(void);

/* Return the total number of usable frames */
uint32_t pmm_total_frames(void);

#endif /* PMM_H */
