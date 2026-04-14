#ifndef GDT_H
#define GDT_H

/*
 * AstraOS Global Descriptor Table (GDT)
 *
 * Provides a minimal flat-memory GDT with three descriptors:
 *   Selector 0x00 — null descriptor       (required by x86 architecture)
 *   Selector 0x08 — ring-0 code segment   (base=0, limit=4 GiB, execute/read)
 *   Selector 0x10 — ring-0 data segment   (base=0, limit=4 GiB, read/write)
 *
 * gdt_init() must be called before interrupts_init() so that the IDT's
 * segment selectors (0x08 for gates) reference this kernel-owned table.
 */

/* Initialise and load the kernel GDT, then reload all segment registers */
void gdt_init(void);

#endif /* GDT_H */
