/*
 * AstraOS Kernel Entry Point — v0.2
 *
 * Boot sequence:
 *  1. VGA + Serial init  (dual-output from the first instruction onward)
 *  2. Multiboot magic validation
 *  3. IDT + PIC init     (CPU exceptions and hardware IRQs fully handled)
 *  4. Physical Memory Manager (parsed from Multiboot memory map)
 *  5. PIT timer          (100 Hz, global tick counter)
 *  6. PS/2 Keyboard      (IRQ1, scancode → ASCII, circular buffer)
 *  7. STI                (hardware interrupts enabled)
 *  8. Self-test          (provably live: shows uptime, echoes keystrokes)
 */

#include "kernel.h"
#include "kprintf.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/pit.h"
#include "../drivers/keyboard.h"
#include "../memory/memory.h"
#include "../memory/pmm.h"
#include "../interrupts/interrupts.h"

/*
 * kernel_end is defined by the linker script — it marks the first byte
 * after the kernel image in physical memory, used by the PMM to avoid
 * allocating frames that the kernel occupies.
 */
extern uint32_t kernel_end;

/* ------------------------------------------------------------------ */
/*  Panic                                                               */
/* ------------------------------------------------------------------ */

void kernel_panic(const char *msg)
{
    interrupts_disable();
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_puts("\n\n*** KERNEL PANIC ***\n");
    vga_puts(msg);
    vga_puts("\nSystem halted.\n");

    serial_puts("\n\n*** KERNEL PANIC ***\n");
    serial_puts(msg);
    serial_puts("\nSystem halted.\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

/* ------------------------------------------------------------------ */
/*  Boot banner                                                         */
/* ------------------------------------------------------------------ */

static void print_banner(void)
{
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    kputs("    ___        _               ___  ____  \n");
    kputs("   / _ \\  ___ | |_  _ __  __ _/ _ \\/ ___| \n");
    kputs("  | |_| |/ __|| __|| '__|/ _` | | | \\___ \\ \n");
    kputs("  |  _  |\\__ \\| |_ | |  | (_| | |_| |___) |\n");
    kputs("  |_| |_||___/ \\__||_|   \\__,_|\\___/|____/ \n\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    kputs("  AstraOS v0.2  -  Hardened Bare-Metal Kernel\n");
    kputs("  =============================================\n\n");
}

/* ------------------------------------------------------------------ */
/*  Subsystem status line helper                                        */
/* ------------------------------------------------------------------ */

static void status_ok(const char *name)
{
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kputs("  [OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kputs(name);
    kputs("\n");
}

/* ------------------------------------------------------------------ */
/*  CPU identification via CPUID                                        */
/* ------------------------------------------------------------------ */

static void print_cpu_info(void)
{
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];

    /* CPUID leaf 0: vendor string */
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );

    /* Vendor string is in EBX, EDX, ECX (in that order) */
    vendor[0]  = (char)( ebx        & 0xFF);
    vendor[1]  = (char)((ebx >>  8) & 0xFF);
    vendor[2]  = (char)((ebx >> 16) & 0xFF);
    vendor[3]  = (char)((ebx >> 24) & 0xFF);
    vendor[4]  = (char)( edx        & 0xFF);
    vendor[5]  = (char)((edx >>  8) & 0xFF);
    vendor[6]  = (char)((edx >> 16) & 0xFF);
    vendor[7]  = (char)((edx >> 24) & 0xFF);
    vendor[8]  = (char)( ecx        & 0xFF);
    vendor[9]  = (char)((ecx >>  8) & 0xFF);
    vendor[10] = (char)((ecx >> 16) & 0xFF);
    vendor[11] = (char)((ecx >> 24) & 0xFF);
    vendor[12] = '\0';

    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    kprintf("  CPU: %s (max leaf 0x%x)\n", vendor, eax);
}

/* ------------------------------------------------------------------ */
/*  Self-test: prove the OS is live and interactive                     */
/* ------------------------------------------------------------------ */

static void run_selftest(void)
{
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    kputs("\n  === AstraOS Self-Test ===\n");

    /* Test 1: timer is ticking */
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kputs("  [1] Timer ... ");
    uint32_t t0 = pit_get_ticks();
    /* busy-spin briefly with interrupts enabled so timer can fire */
    for (volatile uint32_t i = 0; i < 2000000u; i++) {}
    uint32_t t1 = pit_get_ticks();
    if (t1 > t0) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        kprintf("PASS (ticks advanced %u)\n", t1 - t0);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        kputs("WARN (no ticks observed — IRQ may be masked)\n");
    }

    /* Test 2: PMM allocation */
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kputs("  [2] PMM alloc/free ... ");
    uint32_t frame = pmm_alloc_frame();
    if (frame) {
        pmm_free_frame(frame);
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        kprintf("PASS (frame at 0x%x)\n", frame);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        kputs("FAIL (no free frame)\n");
    }

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kputs("  Self-test complete.\n\n");
}

/* ------------------------------------------------------------------ */
/*  Kernel main                                                         */
/* ------------------------------------------------------------------ */

void kernel_main(uint32_t magic, const multiboot_info_t *mbi)
{
    /* --- Step 1: output drivers ------------------------------------ */
    vga_init();
    serial_init();   /* failure is non-fatal; we fall back to VGA-only */

    /* --- Step 2: validate Multiboot -------------------------------- */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid Multiboot magic — not loaded by a Multiboot bootloader");
    }

    /* --- Step 3: banner ------------------------------------------- */
    print_banner();

    /* --- Step 4: interrupt subsystem (IDT + PIC) ------------------- */
    interrupts_init();
    status_ok("Interrupt subsystem (IDT + 8259A PIC)");

    /* --- Step 5: physical memory manager --------------------------- */
    uint32_t kend = (uint32_t)&kernel_end;
    memory_init(mbi, kend);
    status_ok("Physical Memory Manager");

    /* --- Step 6: CPU info ------------------------------------------ */
    print_cpu_info();

    /* --- Step 7: timer --------------------------------------------- */
    pit_init();
    status_ok("PIT timer (100 Hz)");

    /* --- Step 8: keyboard ------------------------------------------ */
    keyboard_init();
    status_ok("PS/2 keyboard driver");

    /* --- Step 9: enable interrupts ---------------------------------- */
    interrupts_enable();
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kputs("  Interrupts ENABLED\n");

    /* --- Step 10: self-test ---------------------------------------- */
    run_selftest();

    /* --- Step 11: idle loop with live uptime display --------------- */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    kputs("  AstraOS is running.  Type on the keyboard to interact.\n");
    kputs("  Uptime: ");

    uint32_t last_sec = 0;

    for (;;) {
        uint32_t ticks = pit_get_ticks();
        uint32_t sec   = ticks / PIT_FREQUENCY_HZ;

        /* Print uptime every second (serial log only to avoid VGA clutter) */
        if (sec != last_sec) {
            last_sec = sec;
            serial_puts("[TICK] uptime=");
            /* manual decimal print to serial to avoid full kprintf overhead */
            char buf[13]; /* max 10 digits + 's' + '\n' + '\0' */
            int  i = 0;
            uint32_t v = sec;
            if (v == 0) { buf[i++] = '0'; }
            while (v > 0) { buf[i++] = (char)('0' + v % 10); v /= 10; }
            /* reverse */
            for (int a = 0, b = i - 1; a < b; a++, b--) {
                char tmp = buf[a]; buf[a] = buf[b]; buf[b] = tmp;
            }
            buf[i] = 's'; buf[i+1] = '\n'; buf[i+2] = '\0';
            serial_puts(buf);
        }

        /* Echo keyboard input */
        if (keyboard_poll()) {
            char c = keyboard_getchar();
            if (c) {
                vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
                vga_putchar(c);
                serial_putchar(c);
            }
        }

        __asm__ volatile ("hlt");
    }
}

