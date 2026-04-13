# AstraOS

A **real, standalone, bare-metal operating system** built from scratch in C and x86 assembly.

AstraOS runs directly on hardware (or QEMU) without relying on Linux, Windows, or any other host OS at runtime. It is not a theme, not a wrapper, not a distro skin — it is a genuine kernel that takes control of the CPU from the moment the bootloader hands it over.

---

## Current State — v0.1 (Minimal Bootable Kernel)

v0.1 is the first milestone: a minimal kernel that **boots and displays output**.

| Feature | Status |
|---------|--------|
| Bootloader (GRUB/Multiboot1) | ✅ |
| x86 assembly boot stub | ✅ |
| Kernel entry point (C) | ✅ |
| VGA text mode output | ✅ |
| Prints "AstraOS" on boot | ✅ |
| Halt/idle loop | ✅ |
| Memory manager | 🔧 stub |
| Interrupt handling (IDT/PIC) | 🔧 stub |
| Process scheduler | 🔧 stub |
| Virtual filesystem | 🔧 stub |

---

## Repository Structure

```
astra-os/
├── boot/               Assembly boot stub (Multiboot header, stack setup)
├── kernel/             C kernel entry point
├── drivers/            Hardware drivers (VGA text mode)
├── memory/             Memory manager stub (PMM, VMM, heap)
├── interrupts/         Interrupt subsystem stub (IDT, PIC, IRQ handlers)
├── process/            Process scheduler stub
├── fs/                 Virtual filesystem stub
├── docs/               Architecture overview and design notes
├── linker.ld           Linker script (kernel loaded at 1 MiB)
├── grub.cfg            GRUB bootloader menu entry
├── Makefile            Build system
└── README.md           This file
```

---

## Building AstraOS

### Prerequisites

You need an **i686-elf cross-compiler** and GRUB utilities. On a Debian/Ubuntu host:

```bash
# Install QEMU and GRUB tools
sudo apt-get install -y qemu-system-x86 grub-pc-bin grub-common xorriso

# Build the i686-elf cross-compiler (or download a pre-built one)
# See: https://wiki.osdev.org/GCC_Cross-Compiler
```

> **Tip:** The [osdev.org GCC Cross-Compiler guide](https://wiki.osdev.org/GCC_Cross-Compiler) explains how to build `i686-elf-gcc` and `i686-elf-binutils`.

### Build the kernel binary

```bash
make
```

This produces `astra-os.bin` — a 32-bit ELF kernel binary.

### Build a bootable ISO

```bash
make iso
```

This produces `astra-os.iso` — a bootable CD-ROM image with GRUB pre-installed.

### Clean build artefacts

```bash
make clean
```

---

## Running in QEMU

### Graphical window

```bash
make run
```

### Headless / serial console

```bash
make run-serial
```

### Manual launch

```bash
qemu-system-i386 -cdrom astra-os.iso
```

You should see the AstraOS boot banner followed by a list of initialised subsystems.

---

## What You Will See

```
    ___        _               ___  ____
   / _ \  ___ | |_  _ __  __ _/ _ \/ ___|
  | |_| |/ __|| __|| '__|/ _` | | | \___ \
  |  _  |\__ \| |_ | |  | (_| | |_| |___) |
  |_| |_||___/ \__||_|   \__,_|\___/|____/

  AstraOS v0.1  -  Minimal Bootable Kernel
  ==========================================

  [OK] VGA text driver
  [--] Memory manager     (stub)
  [--] Interrupt handler  (stub)
  [--] Process scheduler  (stub)
  [--] Virtual filesystem (stub)

  AstraOS is ready.
```

---

## Roadmap

| Version | Goal |
|---------|------|
| v0.1 | ✅ Boot and print "AstraOS" |
| v0.2 | Physical memory manager + paging |
| v0.3 | IDT, PIC, timer IRQ, keyboard driver |
| v0.4 | Preemptive process scheduler + context switch |
| v0.5 | VFS + initramfs + syscall interface |
| v1.0 | Init process + minimal shell + user mode |

---

## Design Principles

- **Real OS** — controls hardware directly; no host OS at runtime.
- **Minimal first** — each milestone proves a concrete capability.
- **Clean code** — readable C and well-commented assembly.
- **Open tooling** — standard GNU/LLVM toolchain, GRUB, QEMU.
- **Hardware-aware** — designed to run on real x86 hardware, not just QEMU.

---

## References

- [OSDev Wiki](https://wiki.osdev.org/) — the canonical bare-metal OS development reference
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

---

## License

AstraOS is released under the [MIT License](LICENSE).
