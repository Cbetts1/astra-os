/*
 * AstraOS Kernel Entry Point
 *
 * This is the first C code that runs after the assembly boot stub
 * (boot/boot.s) sets up the stack and calls kernel_main().
 *
 * Responsibilities of kernel_main():
 *  1. Validate the Multiboot magic number
 *  2. Initialise the VGA text driver
 *  3. Print the AstraOS banner and system status
 *  4. Initialise stub subsystems (memory, interrupts, scheduler, VFS)
 *  5. Enter the idle halt loop
 */

#include "kernel.h"
#include "../drivers/vga.h"
#include "../memory/memory.h"
#include "../interrupts/interrupts.h"
#include "../process/scheduler.h"
#include "../fs/vfs.h"

/* ------------------------------------------------------------------ */
/*  Panic                                                               */
/* ------------------------------------------------------------------ */

void kernel_panic(const char *msg)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_puts("\n*** KERNEL PANIC ***\n");
    vga_puts(msg);
    vga_puts("\nSystem halted.\n");

    /* Disable interrupts and spin forever */
    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

/* ------------------------------------------------------------------ */
/*  Boot banner helpers                                                 */
/* ------------------------------------------------------------------ */

static void print_banner(void)
{
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("    ___        _               ___  ____  \n");
    vga_puts("   / _ \\  ___ | |_  _ __  __ _/ _ \\/ ___| \n");
    vga_puts("  | |_| |/ __|| __|| '__|/ _` | | | \\___ \\ \n");
    vga_puts("  |  _  |\\__ \\| |_ | |  | (_| | |_| |___) |\n");
    vga_puts("  |_| |_||___/ \\__||_|   \\__,_|\\___/|____/ \n");
    vga_puts("\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("  AstraOS v0.1  -  Minimal Bootable Kernel\n");
    vga_puts("  ==========================================\n\n");
}

static void print_subsystem_status(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("  [OK] VGA text driver\n");
    vga_puts("  [--] Memory manager     (stub)\n");
    vga_puts("  [--] Interrupt handler  (stub)\n");
    vga_puts("  [--] Process scheduler  (stub)\n");
    vga_puts("  [--] Virtual filesystem (stub)\n");
    vga_puts("\n");
}

/* ------------------------------------------------------------------ */
/*  Kernel main                                                         */
/* ------------------------------------------------------------------ */

void kernel_main(uint32_t magic, const multiboot_info_t *mbi)
{
    /* Initialise VGA first so we can report any early errors */
    vga_init();

    /* Verify the bootloader passed the correct Multiboot magic */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid Multiboot magic number - was this loaded by a Multiboot bootloader?");
    }

    /* Suppress unused-parameter warning; mbi will be used in future */
    (void)mbi;

    /* Print the boot banner */
    print_banner();

    /* Initialise stub subsystems */
    memory_init();
    interrupts_init();
    scheduler_init();
    vfs_init();

    /* Report subsystem status */
    print_subsystem_status();

    /* Boot complete */
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("  AstraOS is ready.\n");
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts("  (System halted - no scheduler running yet)\n");

    /* Idle loop - will become the scheduler idle task in future */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
