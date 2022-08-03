#include <kernel.h>
#include <terminal/terminal.h>
#include <interrupts/interrupt.h>
#include <interrupts/isr.h>
#include <io/io.h>
#include <io/irq.h>
#include <io/io.h>
#include <strings/string.h>
#include <interrupts/syscalls.h>
#include <prekernel/prekernel.h>

void kmain()
{
    // Messaggio benvenuto
    printf("\n[SUCCESS] Kernel caricato correttamente \n\n");

    // Inizializzazione dell'IDT (Interrupt descriptor table)
    idt_table_init();

    // Inizizializzazione degli ISR (Interrupt service routine) : Interrupt software
    ISR_Initialize();

    // Inizializzazione degli IRQ (Interrupt request) : Interrupt hardware
    IRQ_Initialize();

    // Aggiunta degli interrupt
    register_syscalls();

    // Abilita gli interrupt
    EnableInterrupts();
}

// Funzione Panic del kernel
void kernelPanic(char *message)
{
    printf(message);
    DisableInterrupts();
    while (1)
        ;
}
