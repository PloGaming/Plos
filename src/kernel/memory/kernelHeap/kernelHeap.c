#include <memory/paging/vmm.h>
#include <memory/pmm/pmm.h>
#include <memory/kernelHeap/kernelHeap.h>

// Variabile che definisce la fine dell'heap del kernel
virtual_addr kheap_endOfHeap = KERNEL_HEAP_START_ADDR;

// Funzione che inizializza l'heap del kernel
bool kheap_init()
{
	// Alloca l'indirizzo iniziale dell'heap
	physical_addr kheapStart_phys = (physical_addr)pmm_alloc_block();

	// Controllo se l'indirizzo é valido
	if (!kheapStart_phys)
		return false;

	// Dopo aver inizializzato il kernel, aumenteremo l'heap di PAGE_SIZE
	kheap_endOfHeap += PAGE_SIZE;

	// Eseguo il map dell'indirizzo del fisico a quello virtuale
	if (!vmm_map((virtual_addr)KERNEL_HEAP_START_ADDR, kheapStart_phys))
		return false;

	printf("[INFO] Kernel Heap inizializzato correttamente\n");

	return true;
}

// Funzione chiamata da kmalloc() quando é necessario aumentare la dimensione dell'
// heap del kernel, ritorna NULL se non c'é piu spazio
bool kheap_enlarge(size_t n)
{
	// Controllo che ci sia ancora spazio
	if (kheap_endOfHeap == KERNEl_HEAP_END_ADDR)
		return false;

	// Calcoliamo quante pagine servono
	int times = n / PAGE_SIZE;

	for (int i = 0; i < times; i++)
	{
		// Alloca altro spazio fisico
		physical_addr heapBlock_phys = (physical_addr)pmm_alloc_block();

		if (!heapBlock_phys)
			return false;

		// Eseguo il map dell'indirizzo del fisico a quello virtuale
		if (!vmm_map((virtual_addr)kheap_endOfHeap, heapBlock_phys))
			return false;

		// Aumentiamo l'heap
		kheap_endOfHeap += PAGE_SIZE;
	}

	return true;
}