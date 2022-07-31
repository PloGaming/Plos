#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct
{
    uint32_t ds;
    uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
    uint32_t interrupt, error;
    uint32_t eip, cs, eflags, esp, ss;

} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers *reg);

void ISR_Master_Handler(Registers *regs);
void ISR_RegisterHandler(int interrupt, ISRHandler handler);
void ISR_InitializeGates();
void ISR_Initialize();

#endif