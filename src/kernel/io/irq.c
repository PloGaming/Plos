#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stddef.h>
#include <terminal/terminal.h>

IRQHandler IRQHandlers[16];

void IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    IRQHandlers[irq] = handler;
}

void IRQ_Handler(Registers *regs)
{
    int irq = regs->interrupt - PIC1_OFFSET;

    if (IRQHandlers[irq] != NULL)
    {
        IRQHandlers[irq](regs);
    }
    else
    {
        print("IRQ non gestito!\n");
        char str[3] = {(char)irq, '\n', '\0'};
        print(str);
    }

    PIC_sendEOI(irq);
}

void IRQ_Initialize()
{
    PIC_init();
    for (int i = 0; i < 16; i++)
    {
        ISR_RegisterHandler(PIC1_OFFSET + i, IRQ_Handler);
    }
    EnableInterrupts();
    print("[INFO] PIC e IRQs inizializzati\n");
}
