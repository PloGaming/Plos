#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA 0x21
#define PIC2_COMMAND PIC2
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

void PIC_sendEOI(unsigned char irq);
void PIC_remap(int offset1, int offset2);
void PIC_init();
void PIC_Mask(int irq);
void PIC_Unmask(int irq);
void PIC_Disable(int irq);
uint16_t PIC_GetIRQRequestRegister();
uint16_t PIC_GetIRQServiceRegister();
void PIC_send_command(uint8_t command, uint8_t pic_num);
void PIC_send_data(uint8_t data, uint8_t pic_num);
uint8_t PIC_read_data(uint8_t pic_Num);
#endif