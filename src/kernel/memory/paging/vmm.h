#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory/paging/pageDirectory.h>
#include <memory/paging/pageTable.h>
#include <strings/string.h>

// Typedef per indicare indirizzi virtuali
typedef uint32_t virtual_addr;

// Typedef per indicare indirizzi virtuali
typedef uint32_t physical_addr;

// Costanti definite nell'architettura x86
#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024

// Macro per estrarre le parti da un indirizzo virtuale
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_PHYSICAL_ADDRESS(x) ((*x) & ~0xfff) // ritorna solo l'indirizzo rimuovendo le flags

// In una page table ci sono 1024 entry ed ognuna descrive
// 4096 byte (1024 * 4096 = 4MB di memoria)
#define PAGE_TABLE_ADDR_SPACE_SIZE 0x400000

// In una page directory ci sono 1024 entry ed ognuna descrive
// 4MB byte (1024 * 4MB = 4GB di memoria)
#define PAGE_DIRECTORY_ADDR_SPACE_SIZE 0x100000000

// Dimensione pagina standard (4kb)
#define PAGE_SIZE 4096

// Strutture per page_table e page_directory
struct page_table
{
	pte entries[PAGES_PER_TABLE];
};

struct page_directory
{
	pde entries[TABLES_PER_DIR];
};

// Funzioni per la vmm
bool vmm_alloc_page(pte *e);
void vmm_free_page(pte *e);
pte *vmm_ptable_get_entry(struct page_table *p, virtual_addr addr);
bool vmm_switch_pdirectory(struct page_directory *dir);
struct page_directory *vmm_get_directory();
void vmm_map_page(void *phys, void *virt);
void vmm_initialize();

// Funzioni utili per gestire il paging
extern uint32_t read_cr0();
extern void set_cr0(uint32_t value);
extern uint32_t read_cr3();
extern void set_cr3(uint32_t value);
extern void vmm_flush_tlb_entry(virtual_addr addr);

#endif