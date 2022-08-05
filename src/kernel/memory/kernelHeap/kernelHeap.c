#include <memory/paging/vmm.h>
#include <memory/pmm/pmm.h>
#include <memory/kernelHeap/kernelHeap.h>

// Funzione che inizializza l'heap del kernel
bool kheap_init()
{
	// Alloca l'indirizzo iniziale dell'heap
	physical_addr kheapStart_phys = (physical_addr)pmm_alloc_block();

	// Controllo se l'indirizzo é valido
	if (!kheapStart_phys)
		return false;

	// Eseguo il map dell'indirizzo del fisico a quello virtuale
	vmm_map((virtual_addr)KERNEL_HEAP_START_ADDR, kheapStart_phys);

	return true;
}