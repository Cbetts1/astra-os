#ifndef PIC_H
#define PIC_H

/*
 * AstraOS 8259A Programmable Interrupt Controller (PIC) Driver
 *
 * The PC/AT uses two cascaded 8259A chips.  By default they map
 * IRQs 0-7 onto CPU vectors 0x08-0x0F, which conflicts with the
 * CPU exception vectors.  This driver remaps them to 0x20-0x2F.
 *
 *   Master PIC: IRQ  0-7  → vector 0x20-0x27
 *   Slave  PIC: IRQ  8-15 → vector 0x28-0x2F
 */

#include <stdint.h>

/* First vector assigned to master/slave IRQs after remapping */
#define PIC_MASTER_OFFSET  0x20   /* 32 */
#define PIC_SLAVE_OFFSET   0x28   /* 40 */

/* Initialise both PICs and remap IRQs to avoid exception conflicts */
void pic_init(void);

/*
 * pic_send_eoi - send End-Of-Interrupt acknowledgement
 * @irq: the IRQ number (0-15) that was serviced
 */
void pic_send_eoi(uint8_t irq);

/*
 * pic_set_mask   - mask (disable) an IRQ line
 * pic_clear_mask - unmask (enable) an IRQ line
 */
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);

/*
 * pic_read_isr - read both PICs' In-Service Register
 *
 * Returns a 16-bit value: bits [7:0] = master ISR, bits [15:8] = slave ISR.
 * Used to detect spurious IRQs before sending EOI.
 */
uint16_t pic_read_isr(void);

#endif /* PIC_H */
