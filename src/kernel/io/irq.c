#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stddef.h>
#include <terminal/terminal.h>
#include <strings/string.h>
#include <memory/memory.h>

static IRQHandler IRQHandlers[16];

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
        printf("IRQ non gestito! %d\n", irq);
    }

    PIC_sendEOI(irq);
}

void IRQ_Initialize()
{
    PIC_init();
    memset(IRQHandlers, '\0', sizeof(IRQHandlers));
    for (int i = 0; i < 16; i++)
    {
        ISR_RegisterHandler(PIC1_OFFSET + i, IRQ_Handler);
    }
    print("[INFO] PIC e IRQs inizializzati\n");
}