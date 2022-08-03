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

// Inizializza la vmm impostando come page directory
// quella impostata precendentemente durante il boot
void vmm_initialize()
{
	extern uint8_t *page_structures;
	struct page_directory *initial_page_dir = (struct page_directory *)&page_structures;

	vmm_switch_pdirectory((struct page_directory *)((void *)initial_page_dir - 0xC0000000));
}