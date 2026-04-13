#ifndef KPRINTF_H
#define KPRINTF_H

/*
 * AstraOS Kernel Printf
 *
 * Minimal formatted output that writes simultaneously to the VGA text
 * console and the COM1 serial port.  This dual-output design means every
 * kernel message is visible on screen AND can be captured from the serial
 * log — making automated CI validation straightforward.
 *
 * Supported format specifiers:
 *   %c  single character
 *   %s  null-terminated string
 *   %d  signed 32-bit decimal
 *   %u  unsigned 32-bit decimal
 *   %x  unsigned 32-bit hexadecimal (lower-case, no "0x" prefix)
 *   %X  unsigned 32-bit hexadecimal (upper-case)
 *   %%  literal percent sign
 */

void kprintf(const char *fmt, ...);

/*
 * kputs - write a string to VGA + serial (no formatting)
 */
void kputs(const char *str);

#endif /* KPRINTF_H */
