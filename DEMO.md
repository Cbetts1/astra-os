# AstraOS — Demo & Proof of Authenticity

This document explains how to run AstraOS yourself and interpret the output
as proof that it is a **real, working bare-metal operating system**.

---

## What makes it "real"?

| Claim | Proof |
|-------|-------|
| Not running on Linux | No Linux kernel, no libc, `-ffreestanding -nostdlib` flags |
| Controls hardware directly | Writes to VGA buffer at `0xB8000`; programs PIC ports `0x20`/`0xA0` |
| Handles CPU interrupts | Loads an IDT with `LIDT`; exceptions caught, not triple-faulting |
| Real timer interrupt | PIT at 100 Hz; uptime counter advances on serial port |
| Real keyboard input | IRQ1 handler; typed characters appear on screen in real-time |
| Boots on real hardware | Passes Multiboot1 spec; valid ELF32 binary loadable by GRUB |
| Reproducible | GitHub Actions CI builds + boots it on every commit (see badge) |

---

## Option A — Automated CI (easiest proof)

Every push to this repository triggers a GitHub Actions workflow that:

1. Installs the i686-linux-gnu cross-compiler
2. Builds `astra-os.bin` from source
3. Verifies the ELF header and Multiboot magic bytes
4. Builds `astra-os.iso` with GRUB
5. Boots the ISO in headless QEMU and checks `AstraOS` appears on serial

See the **Actions** tab for green checkmarks and downloadable artefacts
(`astra-os.bin`, `astra-os.iso`, `serial-boot-log`).

---

## Option B — Build and run yourself (Ubuntu/Debian)

### 1. Install prerequisites

```bash
sudo apt-get update
sudo apt-get install -y \
  gcc-i686-linux-gnu binutils-i686-linux-gnu \
  grub-pc-bin grub-common xorriso \
  qemu-system-x86
```

### 2. Build

```bash
git clone https://github.com/Cbetts1/astra-os.git
cd astra-os
make iso
```

Expected output:
```
i686-linux-gnu-gcc -std=gnu99 -ffreestanding -O2 -Wall -Wextra ... -c kernel/kernel.c
...
i686-linux-gnu-gcc -T linker.ld -ffreestanding -O2 -nostdlib ... -o astra-os.bin ...
grub-mkrescue -o astra-os.iso isodir
```

### 3. Verify the binary (proves it's a real kernel)

```bash
# Confirm it is a 32-bit x86 ELF executable
file astra-os.bin
# → astra-os.bin: ELF 32-bit LSB executable, Intel 80386, ...

# Confirm the Multiboot magic is present
python3 -c "
data = open('astra-os.bin','rb').read()
pos = data.find(b'\x02\xb0\xad\x1b')
print(f'Multiboot magic at 0x{pos:x}')
"
# → Multiboot magic at 0x1000
```

### 4. Boot in QEMU — graphical window

```bash
make run
```

You will see:

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
  CPU: GenuineIntel (max leaf 0x...)
  [OK] PIT timer (100 Hz)
  [OK] PS/2 keyboard driver
  Interrupts ENABLED

  === AstraOS Self-Test ===
  [1] Timer ... PASS (ticks advanced N)
  [2] PMM alloc/free ... PASS (frame at 0x...)
  Self-test complete.

  AstraOS is running.  Type on the keyboard to interact.
```

Type on the keyboard — characters appear in real-time, processed by the
IRQ1 keyboard handler.

### 5. Boot in headless mode with serial capture

```bash
make run-serial
```

All kernel output is mirrored to the serial port.  You will see identical
output in your terminal.  The uptime counter (`[TICK] uptime=Ns`) is
printed to serial every second — proving the PIT timer interrupt is firing.

### 6. Run the CI validation locally

```bash
make test
```

This boots QEMU with a 10-second timeout and checks for the `AstraOS`
string in the serial output:

```
VALIDATION PASS: AstraOS banner found in serial output
```

---

## Option C — Boot on real x86 hardware

1. Write the ISO to a USB drive:
   ```bash
   sudo dd if=astra-os.iso of=/dev/sdX bs=4M status=progress
   ```
   *(replace `/dev/sdX` with your USB device)*

2. Boot the target machine from USB — select the GRUB entry "AstraOS v0.2"

3. You will see the same banner and self-test output on the screen.

> **Note:** Requires legacy BIOS or CSM mode on UEFI systems.
> Secure Boot must be disabled (the kernel is not signed).

---

## What you are seeing

| Output | What it proves |
|--------|---------------|
| VGA banner | Kernel writes directly to hardware at `0xB8000` |
| `[OK] Interrupt subsystem` | IDT loaded (`LIDT` executed), PIC remapped |
| `[OK] Physical Memory Manager` | Multiboot memory map parsed, frames counted |
| `CPU: GenuineIntel` | `CPUID` instruction executed in ring-0 |
| `[OK] PIT timer (100 Hz)` | Port `0x43`/`0x40` programmed; IRQ0 unmasked |
| `[OK] PS/2 keyboard driver` | IRQ1 unmasked; scancode table installed |
| `Self-Test [1] PASS` | Timer interrupt fired between two `pit_get_ticks()` calls |
| `Self-Test [2] PASS` | PMM allocated and freed a physical 4 KiB page frame |
| `[TICK] uptime=Ns` (serial) | PIT IRQ fires 100×/second; C handler increments counter |
| Keyboard echo | PS/2 IRQ1 fires; scancode decoded; character written to VGA |

---

## Source code highlights

| File | What it proves |
|------|---------------|
| `boot/boot.s` | Multiboot header (magic `0x1BADB002`), stack setup, `call kernel_main` |
| `interrupts/isr.s` | 48 assembly stubs that save/restore CPU state around C handlers |
| `interrupts/idt.c` | 256-gate IDT, `LIDT` instruction |
| `interrupts/pic.c` | ICW1-ICW4 sent to ports `0x20`/`0xA0` — hardware PIC programming |
| `drivers/pit.c` | `0x36` → port `0x43`; divisor → port `0x40` |
| `drivers/keyboard.c` | `IRQ1` handler reads from port `0x60` |
| `drivers/serial.c` | 16550 UART init + loopback self-test |
| `memory/pmm.c` | Multiboot memory map parser, 128 KiB bitmap allocator |
| `linker.ld` | Kernel loaded at 1 MiB; `kernel_end` symbol exported |

---

## License

AstraOS is open-source under the [MIT License](LICENSE).
