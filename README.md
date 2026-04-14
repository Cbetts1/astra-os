# AstraOS

**AstraOS** is a unique, bare-metal x86 operating system built from scratch. It features a hardened interrupt subsystem, a physical memory manager, a PIT timer, and a PS/2 keyboard driver — all running in 32-bit protected mode under a Multiboot-compatible bootloader.

## Features

- Multiboot1-compliant boot via GRUB
- VGA text-mode console (80×25, full 16-colour support)
- COM1 serial output for headless/remote testing
- 256-entry IDT with full CPU exception and hardware IRQ coverage
- 8259A dual-PIC remapping (IRQs land on vectors 0x20–0x2F)
- 100 Hz PIT timer with uptime reporting
- PS/2 keyboard driver (US QWERTY, scancode set 1)
- Bitmap-based Physical Memory Manager (4 KiB frames, up to 4 GiB)
- Kernel bump-heap allocator (256 KiB)
- Boot-time self-test: timer tick advance and PMM alloc/free

## Requirements

- **Cross-compiler**: `i686-elf-gcc` (preferred) or `i686-linux-gnu-gcc`
- **Assembler**: matching `i686-elf-as` or `i686-linux-gnu-as`
- **Bootable ISO**: `grub-mkrescue` + `xorriso`
- **Emulator**: `qemu-system-i386`

On Debian/Ubuntu:
```sh
sudo apt install gcc-i686-linux-gnu binutils-i686-linux-gnu \
                 grub-pc-bin xorriso qemu-system-x86
```

## Building

```sh
# Build the kernel binary
make

# Build a bootable ISO image
make iso
```

## Running

```sh
# Launch in QEMU with a graphical window
make run

# Launch in QEMU with serial output to stdout (headless / phone-friendly)
make run-serial

# CI validation: boot, capture serial log, verify AstraOS banner
make test
```

### Quick QEMU command (no Makefile)

```sh
qemu-system-i386 -cdrom astra-os.iso \
  -serial stdio -no-reboot -no-shutdown
```

Add `-display none` for fully headless operation.

## Project Structure

```
boot/          Multiboot1 entry point and stack setup
drivers/       VGA, serial (COM1), PIT timer, PS/2 keyboard
fs/            Virtual Filesystem stub (planned)
interrupts/    IDT, 8259A PIC, ISR/IRQ assembly stubs
kernel/        kernel_main, kprintf, panic
linker.ld      Kernel link layout (loads at 1 MiB)
memory/        Physical Memory Manager + bump-heap
process/       Scheduler stub (planned)
```

## Contributing

Contributions are welcome. Please open an issue or pull request to get started.
