#include <memory/pmm/pmm.h>
#include <memory/paging/vmm.h>
#include <memory/memory.h>
#include <strings/string.h>

// Trasformiamo il size (in kb) in byte e poi lo dividiamo per BLOCK_SIZE
#define KB_TO_BLOCKS(x) (((x)*1024) / PMM_BLOCK_SIZE)

// Macro utili per allineare indirizzi
#define IS_ALIGNED(addr, align) !((addr) & ~((align)-1))
#define ALIGN(addr, align) (((addr) & ~((align)-1)) + (align))

static uint32_t pmm_memory_size; // Dimensioni della memoria disponibile in kb
static uint32_t pmm_used_blocks; // Numero di blocchi utilizzati
static uint32_t pmm_max_blocks;	 // Il massimo numero di blocchi di dimensione BLOCK_SIZE
static uint32_t *pmm_memory_map; // Un puntatore ad una struttura bitmap
static uint32_t pmm_bitmap_size; // Il numero di dwords (4 byte) occupate da pmm_memory_map

// Imposta ad 1 nth bit in *pmm_memory_map
static void bitmap_set(uint32_t bit)
{
	pmm_memory_map[bit / 32] |= (1 << (bit % 32));
}

// Imposta a 0 nth bit in *pmm_memory_map
static void bitmap_unset(uint32_t bit)
{
	pmm_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

bool bitmap_test(int bit)
{
	return pmm_memory_map[bit / 32] & (1 << (bit % 32));
}

static int pmm_first_free()
{

	for (uint32_t i = 0; i < (pmm_max_blocks / 32); i++)
	{
		// Se i 4 byte sono tutti pieni vai
		// ai prossimi 4

		if (pmm_memory_map[i] != 0xffffffff)
		{
			// Per ogni bit del byte testiamo se è
			// impostato a 0
			for (int j = 0; j < 32; j++)
			{
				int bit = 1 << j;
				// Controlla se non è impostato
				if (!(*(pmm_memory_map + i) & bit))
				{
					// Ritorna la posizione precisa del bit
					return i * 4 * 8 + j;
				}
			}
		}
	}

	// Tutto occupato : (
	return -1;
}

int get_free_blocks()
{
	return pmm_max_blocks - pmm_used_blocks;
}

int get_used_blocks()
{
	return pmm_used_blocks;
}

void *get_memory_map_addr()
{
	return (void *)pmm_memory_map;
}

// Funzione che inizializza il pmm impostando tutta
// la memoria come occupata
void pmm_init(size_t memSize, uint32_t *bitmap)
{
	// Imposta tutte le variabili globali ai valori dati
	pmm_memory_map = (uint32_t *)bitmap;
	pmm_memory_size = memSize; // Valore in kb
	pmm_max_blocks = KB_TO_BLOCKS(memSize);
	pmm_used_blocks = pmm_max_blocks; // Impostiamo tutto come pieno

	pmm_bitmap_size = pmm_max_blocks / 32; // In ogni dword ci sono 32 blocchi

	if (pmm_max_blocks % 32)
	{
		pmm_bitmap_size++; // Se non è alinneato perfettamente aumentiamo il size di 1
	}

	// Imposta tutta la memoria come occupata
	memset((char *)pmm_memory_map, 0xff, pmm_bitmap_size);
}

// inizializza una determinata regione impostandola
// come libera, sara il kernel con la memory map a stabilire
// quali aree di memoria possono essere utilizzate
void pmm_init_region(uint32_t base_addr, size_t size)
{
	size_t blocks = size / PMM_BLOCK_SIZE;
	uint32_t align = base_addr / PMM_BLOCK_SIZE;

	for (; blocks > 0; blocks--)
	{
		bitmap_unset(align++);
		pmm_used_blocks--;
	}

	bitmap_set(0);
}

// Imposta un'intera regione di spazio come occupata
void pmm_deinit_region(uint32_t base_addr, size_t size)
{
	size_t blocks = size / PMM_BLOCK_SIZE;
	uint32_t align = base_addr / PMM_BLOCK_SIZE;

	for (; blocks > 0; blocks--)
	{
		bitmap_set(align++);
		pmm_used_blocks++;
	}
}

// Funzione che inizializza tutte le regioni di memoria che GRUB
// ha impostato come libere
void pmm_init_available_regions(uint32_t _mmap_start, uint32_t _mmap_end)
{
	multiboot_memory_map_t *mmap_start = (multiboot_memory_map_t *)(_mmap_start + HIGHER_HALF_PAGING);
	multiboot_memory_map_t *mmap_end = (multiboot_memory_map_t *)(_mmap_end + HIGHER_HALF_PAGING);

	printf("Elenco regioni di memoria trovati:\n");
	// Loop per ogni regione di memoria
	for (int i = 1; mmap_start < mmap_end; mmap_start++, i++)
	{
		printf("%d_ ", i);

		// Se il tipo della regione è MULTIBOOT_MEMORY_AVAILABLE allora lo inizializziamo
		// (percio lo impostiamo come libero)
		if (mmap_start->type == MULTIBOOT_MEMORY_AVAILABLE)
		{
			printf("Regione LIBERA di memoria trovata ad indirizzo: %x - %x\n", (uint32_t)mmap_start->addr, (uint32_t)mmap_start->addr + (uint32_t)mmap_start->len - 1);
			pmm_init_region((uint32_t)mmap_start->addr, (size_t)mmap_start->len);
			continue;
		}
		printf("Regione RISERVATA di memoria trovata ad indirizzo: %x - %x\n", (uint32_t)mmap_start->addr, (uint32_t)mmap_start->addr + (uint32_t)mmap_start->len - 1);
	}
	printf("\n");
}

// Dobbiamo impostare tutta l'area del kernel come occupata
void pmm_deinit_kernel()
{
	// Simboli esportati dal linker
	extern uint8_t *kernel_start;
	extern uint8_t *kernel_end;

	size_t kernel_size = (size_t)&kernel_end - (size_t)&kernel_start;

	// Allineiamo il bitmap a 4096 byte se non è gia allineato
	uint32_t pmm_bitmap_size_aligned = pmm_bitmap_size;
	if (!IS_ALIGNED(pmm_bitmap_size_aligned, PMM_BLOCK_SIZE))
	{
		pmm_bitmap_size_aligned = ALIGN(pmm_bitmap_size_aligned, PMM_BLOCK_SIZE);
	}

	// Allinea il kernel a 4096 byte se non è gia allineato
	uint32_t kernel_size_aligned = kernel_size;
	if (!IS_ALIGNED(kernel_size_aligned, PMM_BLOCK_SIZE))
	{
		kernel_size_aligned = ALIGN(kernel_size_aligned, PMM_BLOCK_SIZE);
	}

	// Impostiamo come occupato le regioni del kernel e della bitmap
	pmm_deinit_region((uint32_t)&kernel_start, kernel_size_aligned);
}

void *pmm_alloc_block()
{
	if (pmm_used_blocks - pmm_max_blocks <= 0)
	{
		return NULL;
	}

	int index = pmm_first_free();

	if (index == -1)
	{
		return NULL;
	}

	bitmap_set(index);
	pmm_used_blocks++;

	return (void *)(index * PMM_BLOCK_SIZE);
}

void pmm_free_block(void *p)
{
	if (p == NULL)
	{
		return;
	}

	uint32_t p_addr = (uint32_t)p;

	int index = p_addr / PMM_BLOCK_SIZE;

	bitmap_unset(index);
}

int pmm_get_free_block_count()
{
	return pmm_max_blocks - pmm_used_blocks;
}

int pmm_first_free_s(size_t size)
{

	if (size == 0)
		return -1;

	if (size == 1)
		return pmm_first_free();

	for (uint32_t i = 0; i < pmm_max_blocks; i++)
		if (pmm_memory_map[i] != 0xffffffff)
			for (int j = 0; j < 32; j++)
			{

				int bit = 1 << j;
				if (!(pmm_memory_map[i] & bit))
				{

					int startingBit = i * 32;
					startingBit += bit;

					uint32_t free = 0;
					for (uint32_t count = 0; count <= size; count++)
					{

						if (!bitmap_test(startingBit + count))
							free++;

						if (free == size)
							return i * 4 * 8 + j;
					}
				}
			}

	return -1;
}

void *pmm_alloc_blocks(size_t number)
{

	if (pmm_get_free_block_count() <= number)
		return 0;

	int frame = pmm_first_free_s(number);

	if (frame == -1)
		return 0;

	for (uint32_t i = 0; i < number; i++)
		bitmap_set(frame + i);

	uint32_t addr = frame * PMM_BLOCK_SIZE;
	pmm_used_blocks += number;

	return (void *)addr;
}