/*
 * AstraOS Kernel Printf
 *
 * Dual-output formatted print: every message appears on the VGA text
 * console AND on the COM1 serial port simultaneously.
 *
 * Kept intentionally small — no heap allocation, no floating point.
 */

#include "kprintf.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"

#include <stdarg.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static void emit(char c)
{
    vga_putchar(c);
    serial_putchar(c);
}

static void emit_str(const char *s)
{
    if (!s) s = "(null)";
    while (*s) emit(*s++);
}

/* Write unsigned integer in given base (2-16) */
static void emit_uint(uint32_t val, unsigned base, int upper)
{
    const char *digits_lc = "0123456789abcdef";
    const char *digits_uc = "0123456789ABCDEF";
    const char *digits = upper ? digits_uc : digits_lc;
    char buf[32];
    int  i = 0;

    if (val == 0) {
        emit('0');
        return;
    }
    while (val > 0) {
        buf[i++] = digits[val % base];
        val /= base;
    }
    /* buf is in reverse order */
    while (i > 0) {
        emit(buf[--i]);
    }
}

static void emit_int(int32_t val)
{
    if (val < 0) {
        emit('-');
        emit_uint((uint32_t)(-(int64_t)val), 10, 0);
    } else {
        emit_uint((uint32_t)val, 10, 0);
    }
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

void kputs(const char *str)
{
    emit_str(str);
}

void kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            emit(*fmt++);
            continue;
        }
        fmt++; /* consume '%' */
        switch (*fmt) {
        case 'c':
            emit((char)va_arg(ap, int));
            break;
        case 's':
            emit_str(va_arg(ap, const char *));
            break;
        case 'd':
            emit_int(va_arg(ap, int32_t));
            break;
        case 'u':
            emit_uint(va_arg(ap, uint32_t), 10, 0);
            break;
        case 'x':
            emit_uint(va_arg(ap, uint32_t), 16, 0);
            break;
        case 'X':
            emit_uint(va_arg(ap, uint32_t), 16, 1);
            break;
        case '%':
            emit('%');
            break;
        default:
            emit('%');
            emit(*fmt);
            break;
        }
        fmt++;
    }

    va_end(ap);
}
