#include "isr.h"
#include <interrupts/isr.h>
#include <interrupts/interrupt.h>
#include <terminal/terminal.h>
#include <strings/string.h>
#include <stddef.h>
#include <kernel.h>
#include <memory/memory.h>
#include <io/pic.h>

static ISRHandler handlers[256];

char *g_Exceptions[] = {
    "Divide by zero error\n",
    "Debug\n",
    "Non-maskable Interrupt\n",
    "Breakpoint\n",
    "Overflow\n",
    "Bound Range Exceeded\n",
    "Invalid Opcode\n",
    "Device Not Available\n",
    "Double Fault\n",
    "Coprocessor Segment Overrun\n",
    "Invalid TSS\n",
    "Segment Not Present\n",
    "Stack-Segment Fault\n",
    "General Protection Fault\n",
    "Page Fault\n",
    "\n",
    "x87 Floating-Point Exception\n",
    "Alignment Check\n",
    "Machine Check\n",
    "SIMD Floating-Point Exception\n",
    "Virtualization Exception\n",
    "Control Protection Exception\n",
    "\n",
    "\n",
    "\n",
    "\n",
    "\n",
    "\n",
    "Hypervisor Injection Exception\n",
    "VMM Communication Exception\n",
    "Security Exception\n",
    "\n"};

void ISR_Initialize()
{
    memset(handlers, '\0', sizeof(handlers));

    ISR_InitializeGates();
    for (int i = 0; i < 256; i++)
    {
        idt_enable_gate(i);
    }
    idt_disable_gate(0x80);
    print("[INFO] ISR Inizializzati!\n");
}

void ISR_RegisterHandler(int interrupt, ISRHandler handler)
{
    handlers[interrupt] = handler;
    idt_enable_gate(interrupt);
}

void ISR_Master_Handler(Registers *regs)
{
    printf("MASTER HANDLER AT %x\n", ISR_Master_Handler);
    if (handlers[regs->interrupt] != NULL)
    {
        handlers[regs->interrupt](regs);
    }
    else if (regs->interrupt >= 32)
    {
        kernelPanic("Interrupt non gestito!\n");
    }
    else
    {
        print("Eccezione non gestita!\nError: ");
        kernelPanic(g_Exceptions[regs->interrupt]);
    }
}
