#include <memory/paging/vmm.h>
#include <terminal/terminal.h>
#include <memory/pmm/pmm.h>
#include <memory/memory.h>

struct page_directory *current_dir = NULL;

// Alloca una pagina dato un puntatore virtuale ad una pte
bool vmm_alloc_page(pte *entry)
{
	if (!entry)
		return false;

	// Alloca lo spazio per la nostra pte
	physical_addr page_addr = (uint32_t)pmm_alloc_block();

	if (!page_addr)
		return false;

	// Impostiamo il frame fisico
	pte_set_addr(entry, page_addr);

	// Impostiamo le relative flags
	pte_add_flags(entry, x86_PTE_PRESENT | x86_PTE_WRITABLE);

	return true;
}

// Libera una pagina dato un un puntatore virtuale ad una pte
bool vmm_free_page(pte *entry)
{
	if (!entry)
		return false;

	// Deallochiamo il blocco
	pmm_free_block((void *)PAGE_GET_ADDRESS(entry));

	// Imposta il frame e tutte le flag della pte come vuote
	*entry = 0;

	return true;
}

// Restituisce l'indirizzo virtuale di una page directory entry data un entry num (0 - 1023)
pde *vmm_directory_get_entry(uint32_t entry_num)
{
	// Se l'entry_num non é valida ritorniamo un puntatore nullo
	if (entry_num > 1023 || entry_num < 0)
		return NULL;

	uint32_t offset = entry_num * PAGE_SIZE;

	return (pde *)((void *)LAST_4MB_ADDRESS_SPACE + offset);
}

// Restituisce l'indirizzo virtuale di una page table entry data un entry num (0 - 1023) ed una directory entry valida
pte *vmm_table_get_entry(pde *directory_entry, uint32_t entry_num)
{
	// Se l'entry_num non é valida ritorniamo un puntatore nullo
	if (entry_num > 1023 || entry_num < 0 || !directory_entry)
		return NULL;

	// Ogni entry ha una dimensione di 4 byte, percio moltiplichiamo l'index
	// Per questo
	uint32_t offset = entry_num * 4;

	return (pte *)((void *)directory_entry + offset);
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
	printf("Page directory cambiata a: %x\n", read_cr3());

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