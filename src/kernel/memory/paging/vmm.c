#include <memory/paging/vmm.h>
#include <terminal/terminal.h>
#include <memory/pmm/pmm.h>
#include <memory/memory.h>

struct page_directory *current_dir = NULL;

// Funzione che alloca una pagina virtuale
// e la imposta come presente in una
// determinata page Table
bool vmm_alloc_page(pte *page)
{
	// Allochiamo un blocco fisico di memoria
	void *block = pmm_alloc_block();
	if (!block)
	{
		return false;
	}

	// Impostiamo il blocco nella page table entry data
	pte_set_addr(page, (physical_addr)block);

	// Impostiamo la pagina come presente
	pte_add_flags(page, x86_PTE_PRESENT);

	return true;
}

// Funzione che dealloca una pagina
// e la imposta come assente
void vmm_free_page(pte *page)
{
	// Otteniamo l'indirizzo del blocco
	void *block = (void *)PAGE_PHYSICAL_ADDRESS(page);

	// Se l'indirizzo è valido
	if (block)
	{
		// Deallochiamo il blocco nella memoria fisica
		pmm_free_block(block);
	}

	// Lo impostiamo come non presente
	pte_clear_flags(page, x86_PTE_PRESENT);
}

// Funzione che restituisce una determinata page table entry
// data la page table e un indirizzo virtuale
pte *vmm_ptable_get_entry(struct page_table *p, virtual_addr addr)
{
	if (p)
	{
		// Entra all'interno delle entries
		// estrae l'index della pte dall'indirizzo
		// e la usa per ottenere il pte
		return &p->entries[PAGE_TABLE_INDEX(addr)];
	}
	return NULL;
}

// Funzione che restituisce una determinata page directory entry
// data la page directory e un indirizzo virtuale
pde *vmm_pdirectory_get_entry(struct page_directory *p, virtual_addr addr)
{
	if (p)
	{
		// Entra all'interno delle entries
		// estrae l'index della pte dall'indirizzo
		// e la usa per ottenere il pte
		return &p->entries[PAGE_DIRECTORY_INDEX(addr)];
	}
	return NULL;
}

// Funzione che restituisce una page dato un indirizzo virtuale
pte *vmm_get_page(virtual_addr addr)
{
	// Ottieni la page directory corrente
	struct page_directory *pd = current_dir;

	// Ottieni la page table nella page directory
	pde *dir_entry = vmm_pdirectory_get_entry(pd, addr);
	struct page_table *table = (struct page_table *)PAGE_PHYSICAL_ADDRESS(dir_entry);

	// Ottieni la page
	pte *table_entry = vmm_ptable_get_entry(table, addr);

	return table_entry;
}

// Funzione che imposta una page_directory
bool vmm_switch_pdirectory(struct page_directory *dir)
{
	// Controllo che l'indirizzo sia valido
	if (!dir)
	{
		return false;
	}

	current_dir = dir;

	// Impostiamo cr3 con il nuovo indirizzo
	set_cr3((physical_addr)current_dir);
	printf("Page directory cambiata a: %x\n", read_cr3());

	return true;
}

// Funzione che ritorna la page_directory in uso
struct page_directory *vmm_get_directory()
{
	return current_dir;
}

// Funzione che "collega" un frame fisico ad una
// page virtuale, inserendolo all'interno di una page table
void vmm_map_page(void *phys, void *virt)
{
	// Ottieni la page directory
	struct page_directory *pageDir = vmm_get_directory();

	// Ottieni la page directory entry
	pde *entry = vmm_pdirectory_get_entry(pageDir, (virtual_addr)virt);

	// Se la entry nella page directory non esiste dobbiamo
	// crearla prima di usarla
	if ((*entry & x86_PDE_PRESENT) != x86_PDE_PRESENT)
	{
		// Dato che questa entry nella page directory
		// non esiste, non esiste neanche la page table
		// corrispondente, quindi dobbiamo crearla
		struct page_table *table = (struct page_table *)(pmm_alloc_block());

		// Controllo se indirizzo è valido
		if (!table)
		{
			return;
		}

		// "Puliamo l'intera page table"
		memset(table, '\0', sizeof(struct page_table));

		// Prima di inserire la nuova page table dobbiamo
		// creare prima una page directory entry
		pde *directory_entry = vmm_pdirectory_get_entry(pageDir, (virtual_addr)virt);

		// Ora modifichiamo questa pde
		pde_add_flags(directory_entry, x86_PDE_PRESENT | x86_PDE_WRITABLE);

		// La pde deve puntare alla nostra pte
		pde_set_addr(entry, (physical_addr)table);
	}

	// Ottieni la page_table
	struct page_table *table = (struct page_table *)PAGE_PHYSICAL_ADDRESS(entry);

	// Ottieni la page
	pte *table_entry = vmm_ptable_get_entry(table, (virtual_addr)virt);

	// Esegui il map
	pte_set_addr(table_entry, (physical_addr)phys);

	// Imposta la page come presente
	pte_add_flags(table_entry, x86_PTE_PRESENT);
}

// Funzione che "scollega" un frame fisico
// da una page virtuale
void vmm_unmap_page(void *virt_address)
{
	// Otteniamo la page
	pte *page = vmm_get_page((virtual_addr)virt_address);

	// "Pulisce" l'indirizzo
	pte_set_addr(page, 0);

	// Rimuove le flags
	pte_clear_flags(page, x86_PTE_PRESENT);
}

// Inizializza il virtual memory manager ed imposta il paging
bool vmm_initialize()
{
	extern uint8_t *kernel_start;

	// Crea una page directory di default
	struct page_directory *dir = (struct page_directory *)pmm_alloc_blocks(3);
	if (!dir)
		return false;

	// Azzera la nuova page directory
	memset(dir, '\0', sizeof(struct page_directory));

	// Imposta tutta la page_directory come presente e scrivibile
	for (int i = 0; i < 1024; i++)
	{
		dir->entries[i] = x86_PDE_WRITABLE | x86_PDE_PRESENT;
	}

	// Alloca una page_table di default (0MB - 4MB)
	struct page_table *table = (struct page_table *)pmm_alloc_block();
	if (!table)
		return false;

	// Alloca una page_table a 3Gb (Higher-Half kernel)
	struct page_table *table3G = (struct page_table *)pmm_alloc_block();
	if (!table3G)
		return false;

	memset(table, '\0', sizeof(struct page_table));
	memset(table3G, '\0', sizeof(struct page_table));

	// I primi 4 MB (percio una page_table) sono identity mapped
	for (int i = 0, frame = 0x00, virt = 0x00; i < 1024; i++, frame += PAGE_SIZE, virt += PAGE_SIZE)
	{
		// Crea una nuova pte
		pte page = 0;

		// Imposta la pte come presente e scrivibile
		pte_add_flags(&page, x86_PTE_PRESENT | x86_PTE_WRITABLE);

		// Imposta l'indirizzo della pte come quello fisico
		pte_set_addr(&page, frame);

		table3G->entries[PAGE_TABLE_INDEX(virt)] = page;
	}

	// Imposta 4 mb a partire da 3 GB fino a 3 GB + 4MB
	for (int i = 0, frame = (uint32_t)&kernel_start, virt = 0xC0000000; i < 1024; i++, frame += PAGE_SIZE, virt += PAGE_SIZE)
	{
		// Crea una nuova pte
		pte page = 0;

		// Imposta la pte come presente
		pte_add_flags(&page, x86_PTE_PRESENT);

		// Imposta l'indirizzo della pte come quello fisico
		pte_set_addr(&page, frame);

		table->entries[PAGE_TABLE_INDEX(virt)] = page;
	}

	// Map dei 3gb (kernel)
	pde *entry = &dir->entries[PAGE_DIRECTORY_INDEX(0xC0000000)];
	pde_add_flags(entry, x86_PDE_PRESENT | x86_PDE_WRITABLE);
	pde_set_addr(entry, (physical_addr)table);

	// Map dei primi 4 MB
	pde *entry3G = &dir->entries[PAGE_DIRECTORY_INDEX(0x00000000)];
	pde_add_flags(entry3G, x86_PDE_PRESENT | x86_PDE_WRITABLE);
	pde_set_addr(entry3G, (physical_addr)table3G);

	// Cambiamo directory
	vmm_switch_pdirectory(dir);

	// Impostiamo il paging come attivo
	set_cr0(read_cr0() | 0x80000000);
	return true;
}