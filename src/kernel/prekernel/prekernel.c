#include <prekernel/prekernel.h>
#include <kernel.h>
#include <terminal/terminal.h>
#include <memory/pmm/pmm.h>
#include <memory/paging/vmm.h>
#include <strings/string.h>
#include <memory/paging/vmm.h>

void prekernel(multiboot_info_t *boot_info)
{

	extern uint8_t *kernel_start; // Indirizzo dove il kernel inizia
	extern uint8_t *kernel_end;	  // Indirizz dove il kernel finisce
	size_t kernel_size = (size_t)&kernel_end - (size_t)&kernel_start;
	uint32_t memSize = 1024 + boot_info->mem_upper; // Totale memoria sul dispositivo in kb

	// Inizializzazione dello schermo
	terminal_init();

	// Controllo se GRUB non ha avuto problemi
	if (!(boot_info->flags >> 6 & 0x1))
	{
		DisableInterrupts();
		kernelPanic("Memory map invalida : ( \n");
	}

	// Print della memory map
	print_system_information(boot_info);

	// Inizializzazione del pmm (Physical memory manager)
	pmm_init(memSize, (uint32_t *)(0xC0000000 + kernel_size));
	pmm_init_available_regions(boot_info->mmap_addr, boot_info->mmap_addr + boot_info->mmap_length);
	pmm_deinit_region(0x00, 0x0400000);

	// Inizializzazione virtual memory manager
	vmm_initialize();

	// Chiamata al kernel main
	kmain();
}

// Funzione che mostra la memory map del dispositivo
void print_system_information(multiboot_info_t *boot_info)
{
	extern uint8_t *kernel_start; // Indirizzo dove il kernel inizia
	extern uint8_t *kernel_end;	  // Indirizz dove il kernel finisce
	size_t kernel_size = (size_t)&kernel_end - (size_t)&kernel_start;
	uint32_t memSize = 1024 + boot_info->mem_upper; // Totale memoria sul dispositivo in kb

	printf("Elenco informazioni: \n");
	printf("Indirizzo kernel inizio: %x\nIndirizzo kernel fine: %x\nDimensioni kernel: %d byte\n", &kernel_start, &kernel_end, kernel_size);
	printf("RAM totale installata: %dkb\n\n", memSize);
}
