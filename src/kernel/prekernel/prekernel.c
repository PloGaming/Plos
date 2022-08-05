#include <prekernel/prekernel.h>
#include <kernel.h>
#include <terminal/terminal.h>
#include <memory/pmm/pmm.h>
#include <memory/paging/vmm.h>
#include <strings/string.h>

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

	// Inizializzazione del pmm (Physical memory manager)
	pmm_init(memSize, (uint32_t *)(HIGHER_HALF_PAGING + kernel_size));
	pmm_init_available_regions(boot_info->mmap_addr, boot_info->mmap_addr + boot_info->mmap_length);
	pmm_deinit_region(0x00, 0x0400000);

	// Inizializzazione vmm
	vmm_initialize();

	// Chiamata al kernel main
	kmain(boot_info);
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
	printf("RAM totale installata: %dkb\n", memSize);
}

void print_memory_map(multiboot_info_t *boot_info)
{
	multiboot_memory_map_t *mmap_start = (multiboot_memory_map_t *)(boot_info->mmap_addr + HIGHER_HALF_PAGING);
	multiboot_memory_map_t *mmap_end = (multiboot_memory_map_t *)((boot_info->mmap_addr + boot_info->mmap_length) + HIGHER_HALF_PAGING);

	printf("Elenco regioni di memoria trovati:\n");

	// Loop per ogni regione di memoria
	for (int i = 1; mmap_start < mmap_end; mmap_start++, i++)
	{
		printf("%d_ ", i);

		if (mmap_start->type == MULTIBOOT_MEMORY_AVAILABLE)
		{
			printf("Regione LIBERA di memoria trovata ad indirizzo: %x - %x\n", (uint32_t)mmap_start->addr, (uint32_t)mmap_start->addr + (uint32_t)mmap_start->len - 1);
			continue;
		}
		printf("Regione RISERVATA di memoria trovata ad indirizzo: %x - %x\n", (uint32_t)mmap_start->addr, (uint32_t)mmap_start->addr + (uint32_t)mmap_start->len - 1);
	}
}