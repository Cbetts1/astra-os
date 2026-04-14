/*
 * AstraOS Global Descriptor Table (GDT)
 *
 * Installs a minimal flat-memory GDT with three descriptors:
 *   0x00 — null descriptor       (required by x86 architecture)
 *   0x08 — ring-0 code segment   (execute / read, base=0, limit=4 GiB)
 *   0x10 — ring-0 data segment   (read / write, base=0, limit=4 GiB)
 *
 * All segment registers are reloaded after lgdt so the CPU uses the
 * kernel-owned table rather than whatever GRUB left behind.
 *
 * Reference: Intel SDM Vol. 3A §3.4, §3.5
 */

#include "gdt.h"
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  GDT structures                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t limit_low;     /* segment limit bits [15:0]               */
    uint16_t base_low;      /* base address bits  [15:0]               */
    uint8_t  base_mid;      /* base address bits  [23:16]              */
    uint8_t  access;        /* type, S, DPL, P fields                  */
    uint8_t  granularity;   /* limit bits [19:16] + flags (G, D/B, L)  */
    uint8_t  base_high;     /* base address bits  [31:24]              */
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;         /* size of GDT in bytes minus 1            */
    uint32_t base;          /* linear address of the GDT               */
} __attribute__((packed)) gdt_ptr_t;

/* ------------------------------------------------------------------ */
/*  Descriptor access byte values                                       */
/*                                                                      */
/*  Bit 7  (P)  : Segment present = 1                                  */
/*  Bits 6:5 (DPL): Descriptor privilege level                         */
/*  Bit 4  (S)  : Descriptor type (1 = code/data, 0 = system)         */
/*  Bits 3:0    : Segment type                                          */
/*    Code: execute/read = 0x0A  (E=1, C=0, RW=1, A=0)                */
/*    Data: read/write   = 0x02  (E=0, ED=0, RW=1, A=0)               */
/* ------------------------------------------------------------------ */

#define ACCESS_PRESENT  0x80
#define ACCESS_RING0    0x00
#define ACCESS_SEG      0x10   /* S=1: code/data descriptor            */
#define ACCESS_EXEC     0x08   /* code segment (executable)            */
#define ACCESS_RW       0x02   /* readable code / writable data        */

#define ACCESS_CODE  (ACCESS_PRESENT | ACCESS_RING0 | ACCESS_SEG | ACCESS_EXEC | ACCESS_RW)
#define ACCESS_DATA  (ACCESS_PRESENT | ACCESS_RING0 | ACCESS_SEG | ACCESS_RW)

/*
 * Granularity / flags byte:
 *   Bit 7 (G)  : 1 = limit is in 4 KiB pages
 *   Bit 6 (D/B): 1 = 32-bit protected mode
 *   Bit 5 (L)  : 0 (long mode disabled)
 *   Bits 3:0   : Limit bits [19:16] = 0xF (full 4 GiB)
 */
#define GRAN_4K_32BIT  0xCF

/* ------------------------------------------------------------------ */
/*  GDT table (3 entries)                                              */
/* ------------------------------------------------------------------ */

#define GDT_ENTRIES  3

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;

/* ------------------------------------------------------------------ */
/*  Helper — fill one GDT entry                                        */
/* ------------------------------------------------------------------ */

static void gdt_set_entry(int idx, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran)
{
    gdt[idx].limit_low   = (uint16_t)(limit & 0xFFFF);
    gdt[idx].base_low    = (uint16_t)(base  & 0xFFFF);
    gdt[idx].base_mid    = (uint8_t )((base >> 16) & 0xFF);
    gdt[idx].access      = access;
    gdt[idx].granularity = (uint8_t)((gran & 0xF0) | ((limit >> 16) & 0x0F));
    gdt[idx].base_high   = (uint8_t )((base >> 24) & 0xFF);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void gdt_init(void)
{
    /* Descriptor 0: null (required by x86 architecture) */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Descriptor 1: kernel code segment — selector 0x08 */
    gdt_set_entry(1, 0x00000000, 0xFFFFFFFF, ACCESS_CODE, GRAN_4K_32BIT);

    /* Descriptor 2: kernel data segment — selector 0x10 */
    gdt_set_entry(2, 0x00000000, 0xFFFFFFFF, ACCESS_DATA, GRAN_4K_32BIT);

    gdt_ptr.limit = (uint16_t)(sizeof(gdt) - 1);
    gdt_ptr.base  = (uint32_t)&gdt;

    /*
     * Load the GDTR and reload all segment registers.
     * A far jump (ljmp) flushes the instruction pipeline and reloads CS
     * with the new kernel code selector (0x08).
     */
    __asm__ volatile (
        "lgdt  (%0)\n\t"
        "ljmp  $0x08, $1f\n\t"    /* far jump reloads CS = 0x08 */
        "1:\n\t"
        "movl  $0x10, %%eax\n\t"  /* kernel data selector        */
        "movw  %%ax,  %%ds\n\t"
        "movw  %%ax,  %%es\n\t"
        "movw  %%ax,  %%fs\n\t"
        "movw  %%ax,  %%gs\n\t"
        "movw  %%ax,  %%ss\n\t"
        : : "r"(&gdt_ptr) : "eax", "memory"
    );
}
