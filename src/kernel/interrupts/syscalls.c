#include <interrupts/syscalls.h>
#include <interrupts/isr.h>
#include <io/irq.h>

extern uint32_t get_cr2();

void timer(Registers *regs)
{
	return;
}

void keyPress(Registers *regs)
{
	printf("Keyboard Pressed\n");
}

void page_fault_handler(Registers *regs)
{
	printf("Page fault : (\n");
	printf("Error Code: %b\n", regs->error);
	printf("Fault address: %x\n", get_cr2());
	kernelPanic("Exit..\n");
}

void register_syscalls()
{
	IRQ_RegisterHandler(0, timer);
	IRQ_RegisterHandler(1, keyPress);
	ISR_RegisterHandler(14, page_fault_handler);
}