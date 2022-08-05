#include <memory/paging/vmm.h>
#include <memory/pmm/pmm.h>
#include <memory/kernelHeap/kernelHeap.h>
#include <memory/memory.h>

// Strutture heap
struct heap_table currentHeap_table;
struct heap currentHeap;

// Array che descrive i blocchi dell'heap
uint8_t entries[HEAP_SIZE / HEAP_BLOCK_SIZE];

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

	// Inizializziamo la nostra heap table
	currentHeap_table.entries = (virtual_addr *)entries;
	currentHeap_table.total_entries = HEAP_SIZE / HEAP_BLOCK_SIZE;

	// Inizializziamo il nostro heap
	currentHeap.heap_start = (virtual_addr *)KERNEL_HEAP_START_ADDR;
	currentHeap.table = &currentHeap_table;

	// Impostiamo tutte le entry come libere
	memset(entries, IS_FREE, sizeof(entries));

	return true;
}

// Funzione chiamata da kmalloc() quando é necessario aumentare la dimensione dell'
// heap del kernel, ritorna NULL se non c'é piu spazio
bool kheap_enlarge(size_t n)
{
	// Controllo che ci sia ancora spazio
	if (kheap_endOfHeap == KERNEl_HEAP_END_ADDR)
		return false;

	int times = get_blocks(n);

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

		// Imposta il blocco come libero
		int free = (kheap_endOfHeap - PAGE_SIZE - KERNEL_HEAP_START_ADDR) / HEAP_BLOCK_SIZE;
		entries[free] = IS_FREE;

		// Ogni volta che finisce il ciclo controlliamo sempre che
		// abbiamo ancora spazio
		if (kheap_endOfHeap == KERNEl_HEAP_END_ADDR)
			return false;
	}

	return true;
}

// Funzione che restituisce il numero di blocchi necessari
// in base alla dimensione
int get_blocks(size_t n)
{
	int total = n / HEAP_BLOCK_SIZE;

	if (n % HEAP_BLOCK_SIZE != 0)
		total++;

	return total;
}

// Funzione che cerca una serie continua di blocchi pari
// al numero total_blocks, se non presente ritorna -1
int find_free_blocks(int total_blocks)
{
	int found_free_blocks = 0;
	int index = -1;

	for (int i = 0; i < (kheap_endOfHeap - KERNEL_HEAP_START_ADDR) / HEAP_BLOCK_SIZE; i++)
	{
		// Se la entry é occupata
		if ((entries[i] & IS_FREE) != 0b00100000)
		{
			// Significa che il numero di blocchi liberi continui non é sufficiente
			// dovremo trovare un altra serie, reimpostiamo tutto
			index = -1;
			found_free_blocks = 0;
			continue;
		}

		// Se il blocco libero é il primo della lista
		// lo impostiamo come tale
		if (index == -1)
			index = i;

		// Se la entry é libera aggiungiamola alla lista
		found_free_blocks++;

		// Se il numero di blocchi free continui coincide con quelli
		// necessari usciamo dal ciclo
		if (found_free_blocks == total_blocks)
		{
			break;
		}
	}

	if (found_free_blocks != total_blocks)
		index = -1;

	return index;
}

// Funzione che imposta una serie di blocchi come occupati
void mark_series_of_blocks(int first_block, int total_blocks)
{
	for (int i = first_block; i < first_block + total_blocks; i++)
	{
		// Imposta la entry come occupata
		entries[i] = 0x00;

		// Se la entry é la prima della serie allora
		// la imposto come prima
		if (i == first_block)
			entries[i] |= IS_FIRST;

		// Se questo blocco non é l'ultimo della serie
		// imposto il bit HAS_N
		if (i < (first_block + total_blocks - 1))
			entries[i] |= HAS_N;
	}
}

// Funzione che calcola l'indirizzo virtuale del blocco
void *calculate_addr(int block_index)
{
	return (void *)(KERNEL_HEAP_START_ADDR + (block_index * HEAP_BLOCK_SIZE));
}

// Funzione che imposta una serie di blocchi come liberi
void mark_series_of_block_free(int start_block_index)
{
	bool loop = true;
	int i = start_block_index;
	while (loop)
	{
		loop = false;

		// Controlliamo se la entry ha dei successivi blocchi
		if (entries[i] & HAS_N)
			loop = true;

		// Impostiamo la entry come vuota
		entries[i] = IS_FREE;

		// Andiamo alla prossima entry
		i++;
	}
}

// Funzione malloc() per il kernel
// alloca n bytes e ritorna un indirizzo virtuale
// che punta all'inizio del blocco
void *kmalloc(int n)
{
	// Se il size é invalido non andiamo oltre
	if (n <= 0)
		return NULL;

	// Calcoliamo il numero di blocchi necessari per soddisfare la richiesta
	int total_blocks = get_blocks(n);

	// Troviamo la prima serie di blocchi consecutivi abbastanza grande
	int first_block = find_free_blocks(total_blocks);

	// Se l'heap é troppo piccolo, lo aumentiamo fin quando non
	// é abbastanza grande per contenere il contenuto
	while (first_block == -1)
	{
		// Se kheap_enlarge fallisce vuol dire che la memoria é finita
		if (!kheap_enlarge(PAGE_SIZE))
		{
			return NULL;
		}

		first_block = find_free_blocks(total_blocks);
	}

	// Imposta i blocchi come occupati
	mark_series_of_blocks(first_block, total_blocks);

	// Calcola l'indirizzo
	void *addr = calculate_addr(first_block);

	return addr;
}

// Funzione che oltre ad invocare kmalloc() fornisce informazioni di debug aggiuntive
void *kmalloc_debug(int n)
{
	void *addr = kmalloc(n);

	printf("[DEBUG] Indirizzo generato da kmalloc() %x, kernelHeap fine: %x\n", addr, kheap_endOfHeap);

	return addr;
}

// Funzione che libera un blocco di memoria
void kfree(void *addr)
{
	// Controllo che l'indirizzo sia l'inizio di un blocco
	if ((uint32_t)addr % HEAP_BLOCK_SIZE != 0)
		return;

	// Calcoliamo il blocco in base all'indirizzo
	int block_index = ((uint32_t)addr - KERNEL_HEAP_START_ADDR) / HEAP_BLOCK_SIZE;

	// Impostiamo tutti i blocchi come liberi
	mark_series_of_block_free(block_index);
}