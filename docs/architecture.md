# AstraOS Architecture Overview

## What Is AstraOS?

AstraOS is a standalone, bare-metal operating system written in C and x86 assembly. It runs directly on hardware (or in QEMU) without depending on Linux, Windows, or any other host OS at runtime.

## Current State — v0.1 (Minimal Bootable Kernel)

The v0.1 release proves that AstraOS is a real OS by:

- Booting through GRUB using the Multiboot1 protocol
- Setting up a kernel stack in assembly before entering C
- Initialising the VGA text driver and printing **AstraOS** on screen
- Calling stub initialisers for memory, interrupts, scheduler, and VFS
- Entering a safe halt/idle loop

## Boot Flow

```
BIOS/UEFI firmware
    └── GRUB (Multiboot1 bootloader)
            └── loads astra-os.bin at 1 MiB
                    └── boot/boot.s  (_start)
                            └── sets up 16 KiB stack
                                    └── kernel/kernel.c  (kernel_main)
                                            ├── vga_init()
                                            ├── print_banner()
                                            ├── memory_init()   [stub]
                                            ├── interrupts_init() [stub]
                                            ├── scheduler_init() [stub]
                                            ├── vfs_init()      [stub]
                                            └── idle loop (HLT)
```

## Directory Layout

```
astra-os/
├── boot/
│   └── boot.s              Multiboot header + entry point (_start)
├── kernel/
│   ├── kernel.c            C kernel entry point (kernel_main)
│   └── kernel.h            Kernel types and declarations
├── drivers/
│   ├── vga.c               VGA text mode driver (80×25, 16 colours)
│   └── vga.h
├── memory/
│   ├── memory.c            PMM / VMM / heap stub
│   └── memory.h
├── interrupts/
│   ├── interrupts.c        IDT / PIC stub
│   └── interrupts.h
├── process/
│   ├── scheduler.c         Round-robin scheduler stub
│   └── scheduler.h
├── fs/
│   ├── vfs.c               Virtual filesystem stub
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

## Planned Subsystems

### Memory Manager (Phase 1 → v0.2)
- Bitmap physical frame allocator
- Identity-mapped page directory
- Kernel heap (kmalloc / kfree)

### Interrupt Handling (Phase 2 → v0.3)
- 256-entry IDT with CPU exception handlers
- 8259A PIC remapping (IRQs at vectors 0x20–0x2F)
- Timer (IRQ 0) and keyboard (IRQ 1) drivers

### Process Scheduler (Phase 3 → v0.4)
- Process Control Block (PCB)
- Preemptive round-robin scheduling via timer IRQ
- Context switch in assembly

### Virtual Filesystem (Phase 4 → v0.5)
- VFS abstraction layer
- initramfs for early userland
- ext2 driver for persistent storage

### Userland (Phase 5 → v1.0)
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
