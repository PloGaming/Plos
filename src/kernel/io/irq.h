#ifndef IRQ_H
#define IRQ_H
#include <interrupts/isr.h>

typedef void (*IRQHandler)(Registers *regs);

void IRQ_Initialize();
void IRQ_Handler(Registers *regs);
void IRQ_RegisterHandler(int irq, IRQHandler handler);

#endif