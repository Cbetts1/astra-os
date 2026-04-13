# AstraOS

A **real, standalone, bare-metal operating system** built from scratch in C and x86 assembly.

AstraOS runs directly on hardware (or QEMU) without relying on Linux, Windows, or any other
host OS at runtime.  It is not a theme, not a wrapper, not a distro skin — it is a genuine
kernel that takes control of the CPU from the moment the bootloader hands it over.

> **Want proof it's real?** See [DEMO.md](DEMO.md) for step-by-step instructions and the
> CI badge above for automated build+boot verification on every commit.

---

## Current State — v0.2 (Hardened Kernel)

| Feature | Status |
|---------|--------|
| Bootloader (GRUB / Multiboot1) | ✅ |
| x86 assembly boot stub | ✅ |
| VGA text mode driver (80×25, 16 colours) | ✅ |
| COM1 serial driver (115200 baud) | ✅ |
| Dual VGA + serial output (`kprintf`) | ✅ |
| Interrupt Descriptor Table (256 gates) | ✅ |
| 8259A PIC driver (IRQs remapped to 0x20-0x2F) | ✅ |
| CPU exception handlers (all 32 vectors) | ✅ |
| PIT timer (100 Hz, global tick counter) | ✅ |
| PS/2 Keyboard driver (IRQ1, US QWERTY) | ✅ |
| Physical Memory Manager (bitmap, 4 KiB frames) | ✅ |
| Kernel heap bump allocator | ✅ |
| CPUID CPU vendor detection | ✅ |
| Boot self-test suite | ✅ |
| Live keyboard echo | ✅ |
| GitHub Actions CI (build + boot validation) | ✅ |
| Paging / virtual memory | 🔧 next |
| Process scheduler + context switch | 🔧 next |
| Virtual filesystem | 🔧 next |

---

## Repository Structure

```
astra-os/
├── boot/
│   └── boot.s              Multiboot header, stack, _start → kernel_main
├── kernel/
│   ├── kernel.c            Main entry point, subsystem init, idle loop
│   ├── kernel.h            Multiboot structs, public API
│   ├── kprintf.c           Formatted output → VGA + serial simultaneously
│   └── kprintf.h
├── drivers/
│   ├── vga.c / vga.h       Direct VGA text buffer driver (0xB8000)
│   ├── serial.c / serial.h COM1 UART 16550 driver (115200 baud)
│   ├── pit.c / pit.h       PIT 8254 timer (100 Hz, IRQ0)
│   └── keyboard.c / .h     PS/2 keyboard (IRQ1, scancode→ASCII)
├── interrupts/
│   ├── idt.c / idt.h       256-gate IDT, exception+IRQ dispatch
│   ├── isr.s               Assembly stubs for vectors 0-47
│   ├── pic.c / pic.h       8259A PIC remap + EOI
│   ├── interrupts.c / .h   Subsystem init (pic_init + idt_init + sti)
├── memory/
│   ├── pmm.c / pmm.h       Physical memory manager (bitmap allocator)
│   └── memory.c / memory.h Heap + PMM init
├── process/
│   └── scheduler.c / .h    Scheduler stub (round-robin, next milestone)
├── fs/
│   └── vfs.c / vfs.h       VFS stub (initramfs, next milestone)
├── docs/
│   └── architecture.md     Design notes
├── .github/workflows/
│   └── ci.yml              GitHub Actions: build → verify → boot in QEMU
├── linker.ld               Kernel at 1 MiB; exports kernel_end
├── grub.cfg                GRUB menu entry
├── Makefile                Build system (all / iso / run / run-serial / test)
├── DEMO.md                 Step-by-step proof that AstraOS is real
└── README.md               This file
```

---

## Building AstraOS

### Prerequisites

```bash
# Ubuntu / Debian
sudo apt-get install -y \
  gcc-i686-linux-gnu binutils-i686-linux-gnu \
  grub-pc-bin grub-common xorriso \
  qemu-system-x86
```

> Alternatively, build the [i686-elf cross-compiler](https://wiki.osdev.org/GCC_Cross-Compiler)
> for a cleaner bare-metal toolchain.

### Commands

```bash
make          # build kernel ELF binary  →  astra-os.bin
make iso      # build bootable ISO        →  astra-os.iso
make run      # build + launch QEMU (graphical window)
make run-serial  # build + launch QEMU (serial → stdout)
make test     # build + boot + validate serial output
make clean    # remove all build artefacts
```

---

## What You Will See

```
    ___        _               ___  ____
   / _ \  ___ | |_  _ __  __ _/ _ \/ ___|
  | |_| |/ __|| __|| '__|/ _` | | | \___ \
  |  _  |\__ \| |_ | |  | (_| | |_| |___) |
  |_| |_||___/ \__||_|   \__,_|\___/|____/

  AstraOS v0.2  -  Hardened Bare-Metal Kernel
  =============================================

  [OK] Interrupt subsystem (IDT + 8259A PIC)
  [OK] Physical Memory Manager
  CPU: GenuineIntel (max leaf 0x16)
  [OK] PIT timer (100 Hz)
  [OK] PS/2 keyboard driver
  Interrupts ENABLED

  === AstraOS Self-Test ===
  [1] Timer ... PASS (ticks advanced 3)
  [2] PMM alloc/free ... PASS (frame at 0x1234000)
  Self-test complete.

  AstraOS is running.  Type on the keyboard to interact.
```

Type on your keyboard — each character is decoded by the IRQ1 handler and echoed to screen.

---

## Roadmap

| Version | Goal | Status |
|---------|------|--------|
| v0.1 | Boot and print "AstraOS" | ✅ |
| v0.2 | IDT + PIC + PIT + Keyboard + PMM + Serial + CI | ✅ |
| v0.3 | x86 paging, identity-map kernel, enable VMM | 🔧 |
| v0.4 | Preemptive scheduler, context switch, fork/exit | 🔧 |
| v0.5 | VFS + initramfs + syscall table + ring-3 | 🔧 |
| v1.0 | Init process + minimal shell + user mode | 🔧 |

---

## Design Principles

- **Real OS** — controls hardware directly; no host OS at runtime.
- **Provably live** — self-test suite + CI boots it on every commit.
- **Open source** — MIT licensed; every line visible and auditable.
- **Original** — written from scratch; no copy-paste from other kernels.
- **Minimal first** — each milestone proves a concrete new capability.
- **Hardware-aware** — designed to run on real x86 hardware, not just QEMU.

---

## References

- [OSDev Wiki](https://wiki.osdev.org/) — canonical bare-metal OS development reference
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [Intel® 64 and IA-32 SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [8259A PIC datasheet](https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf)
- [8254 PIT datasheet](https://www.scs.stanford.edu/10wi-cs140/pintos/specs/8254.pdf)

---

## License

AstraOS is released under the [MIT License](LICENSE).
