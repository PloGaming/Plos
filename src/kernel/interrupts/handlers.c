#include <interrupts/handlers.h>
#include <interrupts/isr.h>
#include <io/irq.h>
#include <devices/keyboard.h>
#include <devices/pit.h>

extern uint32_t get_cr2();

// Handler per IRQ[0]
// Chiamato quando il timer di sistema
// invia un segnale al PIC
void timer(Registers *regs)
{
	pit_interrupt_handler();
}

// Handler per IRQ[1]
// Chiamato quando un tasto della tastiera
// è premuto
void keyPress(Registers *regs)
{
	keyboard_handler();
}

// Handler per ISR[14]
// Chiamato quando si tenta di accedere ad un
// indirizzo virtuale non mappato
void page_fault_handler(Registers *regs)
{
	printf("Page fault : (\n");
	printf("Error Code: %b\n", regs->error);
	printf("Fault address: %x\n", get_cr2());
	kernelPanic("Exit..\n");
}

// Funzione che registra tutti gli interrupt
void register_syscalls()
{
	IRQ_RegisterHandler(0, timer);
	IRQ_RegisterHandler(1, keyPress);
	ISR_RegisterHandler(14, page_fault_handler);
}