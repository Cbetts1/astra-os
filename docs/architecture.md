# AstraOS Architecture Overview

## What Is AstraOS?

AstraOS is a standalone, bare-metal operating system written in C and x86 assembly. It runs directly on hardware (or in QEMU) without depending on Linux, Windows, or any other host OS at runtime.

## Current State — v0.2 (Hardened Bare-Metal Kernel)

The v0.2 release is a fully functional bare-metal kernel that:

- Boots via GRUB using the Multiboot1 protocol
- Installs a kernel-owned GDT (null, ring-0 code, ring-0 data descriptors)
- Sets up a 256-entry IDT with real exception and IRQ handlers
- Remaps the 8259A dual-PIC to vectors 0x20–0x2F (avoids exception conflicts)
- Handles spurious IRQ7/IRQ15 correctly by reading the PIC In-Service Register
- Drives the PIT at 100 Hz with a global tick counter and a `pit_sleep_ms()` helper
- Supports PS/2 keyboard input via IRQ1 (scancode → ASCII, circular buffer)
- Initialises a 16550 UART (COM1) at 115200 8N1 for serial debug output
- Manages physical memory via a bitmap frame allocator (parsed from Multiboot mmap)
- Provides a 256 KiB bump-pointer kernel heap (`kmalloc` / `kfree`)
- Performs a live self-test (timer ticks, PMM allocation) on every boot

## Boot Flow

```
BIOS/UEFI firmware
    └── GRUB (Multiboot1 bootloader)
            └── loads astra-os.bin at 1 MiB
                    └── boot/boot.s  (_start)
                            └── sets up 16 KiB stack
                                    └── kernel/kernel.c  (kernel_main)
                                            ├── vga_init()
                                            ├── serial_init()
                                            ├── gdt_init()          ← kernel GDT
                                            ├── interrupts_init()   ← PIC + IDT
                                            ├── memory_init()       ← PMM + heap
                                            ├── pit_init()          ← IRQ0, 100 Hz
                                            ├── keyboard_init()     ← IRQ1
                                            ├── interrupts_enable() ← STI
                                            └── idle loop (HLT + keyboard echo)
```

## Directory Layout

```
astra-os/
├── boot/
│   └── boot.s              Multiboot header + entry point (_start)
├── kernel/
│   ├── kernel.c            C kernel entry point (kernel_main, kernel_panic)
│   ├── kernel.h            Kernel types (Multiboot structs) and declarations
│   ├── kprintf.c           Minimal printf-like formatter (kprintf, kputs)
│   ├── kprintf.h
│   ├── gdt.c               Global Descriptor Table — null, ring-0 code/data
│   └── gdt.h
├── drivers/
│   ├── vga.c               VGA text mode driver (80×25, 16 colours)
│   ├── vga.h
│   ├── serial.c            16550 UART driver — 115200 8N1, polling TX
│   ├── serial.h
│   ├── pit.c               PIT channel-0 driver — 100 Hz IRQ0, pit_sleep_ms()
│   ├── pit.h
│   ├── keyboard.c          PS/2 keyboard driver — scancode → ASCII, ring buffer
│   └── keyboard.h
├── memory/
│   ├── memory.c            Bump-pointer kernel heap (kmalloc / kfree)
│   ├── memory.h
│   ├── pmm.c               Bitmap physical frame allocator (4 KiB frames)
│   └── pmm.h
├── interrupts/
│   ├── interrupts.c        Sequenced init: PIC → IDT → unmask cascade
│   ├── interrupts.h
│   ├── isr.s               Assembly stubs for exceptions 0-31 and IRQs 0-15
│   ├── idt.c               IDT install + isr_handler / irq_handler dispatch
│   ├── idt.h               interrupt_frame_t, irq_register_handler()
│   ├── pic.c               8259A PIC driver — remap, EOI, mask, ISR read
│   └── pic.h
├── process/
│   ├── scheduler.c         Round-robin scheduler (stub — requires paging + TSS)
│   └── scheduler.h
├── fs/
│   ├── vfs.c               Virtual filesystem (stub)
│   └── vfs.h
├── docs/
│   └── architecture.md     This document
├── linker.ld               Kernel linker script (loads at 1 MiB)
├── grub.cfg                GRUB menu entry
├── Makefile                Build system
└── README.md               Project overview and build instructions
```

## Memory Map (Physical)

| Region | Address | Description |
|--------|---------|-------------|
| BIOS data | 0x00000 – 0x000FF | BIOS interrupt vectors |
| Conventional | 0x00100 – 0x9FFFF | Usable RAM (640 KiB) |
| VGA buffer | 0xB8000 – 0xBFFFF | Text-mode character cells |
| Kernel | 0x100000+ | AstraOS kernel binary |
| Kernel heap | (static, inside .bss) | 256 KiB bump-pointer allocator |

## Interrupt Architecture

| Vector range | Source | Handler |
|---|---|---|
| 0–31 | CPU exceptions | `isr_handler()` → `kernel_panic()` |
| 32 (IRQ0) | PIT timer | `pit_irq0_handler()` — increments tick counter |
| 33 (IRQ1) | PS/2 keyboard | `keyboard_irq1_handler()` — buffers scancodes |
| 34 (IRQ2) | PIC cascade | Unmasked; required for slave IRQs 8–15 |
| 35–47 (IRQ3–15) | Hardware | Registered via `irq_register_handler()` |

Spurious IRQ7 and IRQ15 are detected by reading the PIC In-Service Register
before dispatching or sending EOI.

## Planned Subsystems

### Virtual Memory + Paging (Phase 3 → v0.3)
- Identity-mapped page directory for the kernel
- Page-fault handler with demand-zero pages
- Virtual address space for future userspace processes

### GDT Extensions (prerequisite for v0.3+)
- TSS descriptor for ring-0 stack on interrupt from ring-3
- Ring-3 code and data descriptors for user processes

### Process Scheduler (Phase 4 → v0.4)
- Process Control Block (PCB)
- Preemptive round-robin scheduling via IRQ0
- Context switch in assembly (requires paging + TSS)

### Virtual Filesystem (Phase 5 → v0.5)
- VFS abstraction layer
- initramfs for early userland
- ext2 driver for persistent storage

### Userland (Phase 6 → v1.0)
- Ring-3 user mode
- System call interface (int 0x80 / SYSCALL)
- init process
- Minimal shell

## Toolchain

| Tool | Purpose |
|------|---------|
| `i686-elf-gcc` | Cross-compiler targeting bare x86 |
| `i686-elf-as` | GNU assembler for x86 assembly |
| `i686-elf-ld` (via gcc) | Linker with custom `linker.ld` |
| `grub-mkrescue` | Produce bootable ISO with GRUB |
| `qemu-system-i386` | Test the ISO without real hardware |
