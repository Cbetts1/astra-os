/*
 * AstraOS 8259A PIC Driver
 *
 * Remaps IRQs to vectors 0x20-0x2F so they don't overlap CPU exceptions,
 * then masks all lines.  Individual drivers unmask the lines they need.
 *
 * Reference: https://wiki.osdev.org/8259_PIC
 */

#include "pic.h"

/* I/O ports */
#define PIC_MASTER_CMD   0x20
#define PIC_MASTER_DATA  0x21
#define PIC_SLAVE_CMD    0xA0
#define PIC_SLAVE_DATA   0xA1

/* Initialisation Command Words */
#define ICW1_INIT     0x10   /* Initialise                          */
#define ICW1_ICW4     0x01   /* ICW4 needed                         */
#define ICW4_8086     0x01   /* 8086/88 mode                        */

/* Commands */
#define PIC_EOI       0x20   /* End-Of-Interrupt                    */

/* ------------------------------------------------------------------ */
/*  Helpers                                                             */
/* ------------------------------------------------------------------ */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* Small delay by writing to an unused port (standard PC technique) */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void pic_init(void)
{
    /* Save existing masks */
    uint8_t mask_master = inb(PIC_MASTER_DATA);
    uint8_t mask_slave  = inb(PIC_SLAVE_DATA);

    /* ICW1: start initialisation sequence, edge-triggered, cascade */
    outb(PIC_MASTER_CMD,  ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC_SLAVE_CMD,   ICW1_INIT | ICW1_ICW4); io_wait();

    /* ICW2: vector offsets */
    outb(PIC_MASTER_DATA, PIC_MASTER_OFFSET); io_wait();
    outb(PIC_SLAVE_DATA,  PIC_SLAVE_OFFSET);  io_wait();

    /* ICW3: cascade wiring (master: slave on IR2; slave: cascade id=2) */
    outb(PIC_MASTER_DATA, 0x04); io_wait(); /* bit 2 = IR2 */
    outb(PIC_SLAVE_DATA,  0x02); io_wait(); /* cascade identity = 2 */

    /* ICW4: 8086 mode */
    outb(PIC_MASTER_DATA, ICW4_8086); io_wait();
    outb(PIC_SLAVE_DATA,  ICW4_8086); io_wait();

    /* Restore saved masks (all lines remain as they were pre-remap) */
    outb(PIC_MASTER_DATA, mask_master);
    outb(PIC_SLAVE_DATA,  mask_slave);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC_SLAVE_CMD, PIC_EOI);
    }
    outb(PIC_MASTER_CMD, PIC_EOI);
}

void pic_set_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t  val;

    if (irq < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }
    val = inb(port) | (uint8_t)(1u << irq);
    outb(port, val);
}

void pic_clear_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t  val;

    if (irq < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }
    val = inb(port) & (uint8_t)~(1u << irq);
    outb(port, val);
}
