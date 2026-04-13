# ============================================================
#  AstraOS Build System
#
#  Requirements:
#    - i686-elf cross-compiler toolchain (gcc, as, ld)
#    - grub-mkrescue + xorriso
#    - qemu-system-i386
#
#  Targets:
#    make          - build the kernel binary
#    make iso      - build a bootable ISO image
#    make run      - build ISO and launch in QEMU (graphical)
#    make run-serial - build ISO and launch in QEMU (serial console)
#    make clean    - remove all build artefacts
# ============================================================

# ---------- Toolchain --------------------------------------------------
# Prefer the bare-metal i686-elf toolchain; fall back to i686-linux-gnu.
ifneq ($(shell which i686-elf-gcc 2>/dev/null),)
  CC := i686-elf-gcc
  AS := i686-elf-as
  LD := i686-elf-gcc
else ifneq ($(shell which i686-linux-gnu-gcc 2>/dev/null),)
  CC := i686-linux-gnu-gcc
  AS := i686-linux-gnu-as
  LD := i686-linux-gnu-gcc
else
  $(error No i686 cross-compiler found. Install i686-elf-gcc or i686-linux-gnu-gcc.)
endif

# ---------- Flags ------------------------------------------------------
CFLAGS  := -std=gnu99 \
           -ffreestanding \
           -O2 \
           -Wall \
           -Wextra \
           -fno-stack-protector \
           -fno-builtin

LDFLAGS := -ffreestanding \
           -O2 \
           -nostdlib \
           -no-pie \
           -Wl,--build-id=none \
           -lgcc

# ---------- Output names -----------------------------------------------
KERNEL_BIN := astra-os.bin
ISO_IMAGE  := astra-os.iso
ISO_DIR    := isodir

# ---------- Source files -----------------------------------------------
C_SOURCES := kernel/kernel.c   \
             drivers/vga.c     \
             memory/memory.c   \
             interrupts/interrupts.c \
             process/scheduler.c \
             fs/vfs.c

ASM_SOURCES := boot/boot.s

OBJECTS := $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)

# ---------- Default target ---------------------------------------------
.PHONY: all
all: $(KERNEL_BIN)

# ---------- Link the kernel --------------------------------------------
$(KERNEL_BIN): $(OBJECTS) linker.ld
	$(LD) -T linker.ld $(LDFLAGS) -o $@ $(OBJECTS)

# ---------- Compile C sources ------------------------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---------- Assemble boot stub -----------------------------------------
%.o: %.s
	$(AS) $< -o $@

# ---------- Build bootable ISO -----------------------------------------
.PHONY: iso
iso: $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN)  $(ISO_DIR)/boot/$(KERNEL_BIN)
	cp grub.cfg       $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)

# ---------- Run in QEMU (graphical window) -----------------------------
.PHONY: run
run: iso
	qemu-system-i386 -cdrom $(ISO_IMAGE)

# ---------- Run in QEMU (serial console - useful for headless CI) ------
.PHONY: run-serial
run-serial: iso
	qemu-system-i386 -cdrom $(ISO_IMAGE) -nographic -serial stdio

# ---------- Clean up ---------------------------------------------------
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(KERNEL_BIN) $(ISO_IMAGE)
	rm -rf $(ISO_DIR)
