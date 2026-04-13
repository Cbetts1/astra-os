#ifndef SERIAL_H
#define SERIAL_H

/*
 * AstraOS Serial Port Driver (COM1 / 16550 UART)
 *
 * Provides character output over the COM1 serial port at 115200 baud.
 * QEMU maps this to stdio when launched with -serial stdio or
 * -serial mon:stdio, making it ideal for CI log capture.
 */

#include <stdint.h>

/* COM1 base I/O port */
#define SERIAL_COM1  0x3F8

/*
 * serial_init - Initialise COM1 at 115200 8N1
 * Returns 1 on success, 0 if the loopback self-test fails.
 */
int  serial_init(void);

/* Write a single character to COM1 */
void serial_putchar(char c);

/* Write a null-terminated string to COM1 */
void serial_puts(const char *str);

#endif /* SERIAL_H */
