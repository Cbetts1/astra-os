/*
 * AstraOS Serial Port Driver (COM1 / 16550 UART)
 *
 * Initialises COM1 to 115200 baud, 8 data bits, no parity, 1 stop bit
 * (8N1).  Uses polling-mode transmit (no IRQ required).
 *
 * Reference: https://wiki.osdev.org/Serial_Ports
 */

#include "serial.h"

/* ------------------------------------------------------------------ */
/*  Register offsets from the base I/O port                            */
/* ------------------------------------------------------------------ */
#define SERIAL_DATA         0  /* Data register (DLAB=0)              */
#define SERIAL_INT_ENABLE   1  /* Interrupt Enable Register (DLAB=0)  */
#define SERIAL_BAUD_LOW     0  /* Baud rate divisor LSB   (DLAB=1)    */
#define SERIAL_BAUD_HIGH    1  /* Baud rate divisor MSB   (DLAB=1)    */
#define SERIAL_FIFO_CTRL    2  /* FIFO Control Register               */
#define SERIAL_LINE_CTRL    3  /* Line Control Register               */
#define SERIAL_MODEM_CTRL   4  /* Modem Control Register              */
#define SERIAL_LINE_STATUS  5  /* Line Status Register                */
#define SERIAL_MODEM_STATUS 6  /* Modem Status Register               */
#define SERIAL_SCRATCH      7  /* Scratch Register                    */

/* Line Control Register bits */
#define LCR_8BITS    0x03   /* 8 data bits                           */
#define LCR_DLAB     0x80   /* Divisor Latch Access Bit              */

/* FIFO Control Register bits */
#define FCR_ENABLE   0x01   /* Enable FIFOs                          */
#define FCR_CLR_RX   0x02   /* Clear receive FIFO                    */
#define FCR_CLR_TX   0x04   /* Clear transmit FIFO                   */
#define FCR_14BYTE   0xC0   /* 14-byte trigger level                 */

/* Modem Control Register bits */
#define MCR_DTR      0x01   /* Data Terminal Ready                   */
#define MCR_RTS      0x02   /* Request To Send                       */
#define MCR_OUT2     0x08   /* IRQ enable (not used in poll mode)    */
#define MCR_LOOP     0x10   /* Loopback mode (self-test)             */

/* Line Status Register bits */
#define LSR_TX_EMPTY 0x20   /* Transmit Holding Register Empty       */
#define LSR_THRE     0x40   /* Transmitter Empty                     */

/* 115200 baud: divisor = 1193180 / (16 * 115200) ≈ 1 */
#define BAUD_115200  1

/* ------------------------------------------------------------------ */
/*  Low-level port I/O helpers                                         */
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

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

int serial_init(void)
{
    /* Disable all interrupts while we configure the UART */
    outb(SERIAL_COM1 + SERIAL_INT_ENABLE, 0x00);

    /* Enable DLAB to set baud rate divisor */
    outb(SERIAL_COM1 + SERIAL_LINE_CTRL, LCR_DLAB);
    outb(SERIAL_COM1 + SERIAL_BAUD_LOW,  BAUD_115200 & 0xFF);
    outb(SERIAL_COM1 + SERIAL_BAUD_HIGH, (BAUD_115200 >> 8) & 0xFF);

    /* 8N1 line control (clears DLAB) */
    outb(SERIAL_COM1 + SERIAL_LINE_CTRL, LCR_8BITS);

    /* Enable and clear FIFOs, 14-byte trigger */
    outb(SERIAL_COM1 + SERIAL_FIFO_CTRL,
         FCR_ENABLE | FCR_CLR_RX | FCR_CLR_TX | FCR_14BYTE);

    /* RTS + DTR + OUT2 */
    outb(SERIAL_COM1 + SERIAL_MODEM_CTRL,
         MCR_DTR | MCR_RTS | MCR_OUT2);

    /* Loopback self-test: send 0xAE and check the echo */
    outb(SERIAL_COM1 + SERIAL_MODEM_CTRL,
         MCR_DTR | MCR_RTS | MCR_OUT2 | MCR_LOOP);
    outb(SERIAL_COM1 + SERIAL_DATA, 0xAE);

    if (inb(SERIAL_COM1 + SERIAL_DATA) != 0xAE) {
        return 0; /* Hardware not present or broken */
    }

    /* Take port out of loopback mode */
    outb(SERIAL_COM1 + SERIAL_MODEM_CTRL,
         MCR_DTR | MCR_RTS | MCR_OUT2);

    return 1;
}

void serial_putchar(char c)
{
    /* Wait until the transmit holding register is empty */
    while (!(inb(SERIAL_COM1 + SERIAL_LINE_STATUS) & LSR_TX_EMPTY)) {
        __asm__ volatile ("pause");
    }
    outb(SERIAL_COM1 + SERIAL_DATA, (uint8_t)c);
}

void serial_puts(const char *str)
{
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r'); /* CR before LF for terminal compat */
        }
        serial_putchar(*str++);
    }
}
