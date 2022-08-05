#include <memory/paging/vmm.h>
#include <terminal/terminal.h>
#include <strings/string.h>
#include <memory/pmm/pmm.h>
#include <memory/memory.h>

struct page_directory *current_dir = NULL;

// Crea ed alloca una pagina ad un indirizzo virtuale (allineato a 4096)
// impostandola come presente e scrivibile
bool vmm_map(virtual_addr addr, physical_addr phys)
{
	// Controllo se addr é allineato
	if (addr % 4096 != 0)
		return false;

	virtual_addr *dir = (virtual_addr *)KERNEL_PAGE_DIR;

	// Ogni page table "copre" 4MB di dati
	int directory_index = addr / PAGE_TABLE_ADDR_SPACE_SIZE;

	// Dividiamo la parte che ci interessa per PAGE_SIZE per ottenere un PTE index
	int table_index = (addr % PAGE_TABLE_ADDR_SPACE_SIZE) / PAGE_SIZE;

	// Controllo se l'index é valido
	if (directory_index < 0 || directory_index > 1023)
		return false;

	if (dir[directory_index] & x86_PDE_PRESENT)
	{
		// La PDE esiste

		// Indirizzo virtuale della page table
		virtual_addr *page_table = (virtual_addr *)vmm_get_table(directory_index);

		if (!(page_table[table_index] & x86_PTE_PRESENT))
		{
			// Page non esiste

			// Impostiamo la pte con l'indirizzo del frame
			// e le flag x86_PTE_PRESENT e x86_PTE_WRITABLE
			page_table[table_index] = phys | 3;
		}
		else
		{
			// La page esiste gia
			return false;
		}
	}
	else
	{
		// La PDE non esiste percio dobbiamo crearla noi

		physical_addr *new_page_table = (physical_addr *)pmm_alloc_block();
		virtual_addr *page_table = (virtual_addr *)vmm_get_table(directory_index);

		// Aggiungi una nuova page table alla page directory
		dir[directory_index] = (physical_addr)new_page_table | 3;

		// Mappiamo la pagina
		page_table[table_index] = phys | 3;
	}

	return true;
}

// Libera la pagina mappata
bool vmm_unmap(virtual_addr virt)
{
	if (virt % 4096 != 0)
		return false;

	virtual_addr *dir = (virtual_addr *)KERNEL_PAGE_DIR;

	// Ogni page table "copre" 4MB di dati
	int directory_index = virt / PAGE_TABLE_ADDR_SPACE_SIZE;

	// Dividiamo la parte che ci interessa per PAGE_SIZE per ottenere un PTE index
	int table_index = (virt % PAGE_TABLE_ADDR_SPACE_SIZE) / PAGE_SIZE;

	if (dir[directory_index] & x86_PDE_PRESENT)
	{
		virtual_addr *page_table = (virtual_addr *)vmm_get_table(directory_index);

		if (page_table[table_index] & x86_PDE_PRESENT)
		{
			// Ora eseguiamo l'unmap della pagina
			page_table[table_index] = 0x2; // r/w, non presente
		}

		int i;

		// Controlla se le altre PTE sono presenti
		for (i = 0; i < 1024; i++)
		{
			if (page_table[i] & x86_PTE_PRESENT)
				break;
		}

		// Se sono tutte vuote allora unmappiamo la pde
		if (i == 1024)
		{
			pmm_free_block((void *)(dir[directory_index] & 0xFFFFF000));
			dir[directory_index] = 2;
		}
	}

	return true;
}

// Alloca una pagina dato un puntatore virtuale ad una pte
bool vmm_alloc_page(virtual_addr *entry)
{
	if (!entry)
		return false;

	// Alloca lo spazio per la nostra pte
	physical_addr page_addr = (physical_addr)pmm_alloc_block();

	if (!page_addr)
		return false;

	// Impostiamo il frame fisico
	pte_set_addr(entry, page_addr);

	// Impostiamo le relative flags
	pte_add_flags(entry, x86_PTE_PRESENT | x86_PTE_WRITABLE);

	return true;
}

// Libera una pagina dato un un puntatore virtuale ad una pte
bool vmm_free_page(virtual_addr *entry)
{
	if (!entry)
		return false;

	// Deallochiamo il blocco
	pmm_free_block((void *)PAGE_GET_ADDRESS(entry));

	// Imposta il frame e tutte le flag della pte come vuote
	*entry = 0;

	return true;
}

// Restituisce l'indirizzo virtuale di una page table data un entry num (0 - 1023)
void *vmm_get_table(uint32_t entry_num)
{
	// Se l'entry_num non é valida ritorniamo un puntatore nullo
	if (entry_num > 1023 || entry_num < 0)
		return NULL;

	uint32_t offset = entry_num * PAGE_SIZE;

	return (void *)LAST_4MB_ADDRESS_SPACE + offset;
}

// Funzione che imposta una page_directory passando per parametro
// l'indirizzo FISICO di essa
bool vmm_switch_pdirectory(struct page_directory *dir)
{
	// Controllo che l'indirizzo sia valido
	if (!dir)
		return false;

	current_dir = dir;

	// Impostiamo cr3 con il nuovo indirizzo
	set_cr3((physical_addr)current_dir->entries);

	return true;
}

// Funzione che ritorna la page_directory in uso
struct page_directory *vmm_get_directory()
{
	return current_dir;
}

// Inizializza la vmm impostando come directory corrente
// Quella del boot
void vmm_initialize()
{
	// Puntatore che si trova alla base della page dir creata durante il boot
	extern uint8_t *page_structures;

	// Page directory iniziale
	struct page_directory *initial_page_dir = (struct page_directory *)&page_structures;

	// Impostiamo la page directory corrente, dato che il paging é attivo sottraiamo
	// 0xC0000000 per ottenere l'indirizzo fisico
	vmm_switch_pdirectory((struct page_directory *)((void *)initial_page_dir - HIGHER_HALF_PAGING));
}