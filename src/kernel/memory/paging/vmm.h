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
// Typedef per indicare indirizzi fisici
typedef uint32_t physical_addr;

// Macro per estrarre le parti da un indirizzo virtuale
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_ADDRESS(x) ((*x) & ~0xfff) // ritorna solo l'indirizzo rimuovendo le flags

// Costanti definite nell'architettura x86
#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024

// In una page table ci sono 1024 entry ed ognuna descrive
// 4096 byte (1024 * 4096 = 4MB di memoria)
#define PAGE_TABLE_ADDR_SPACE_SIZE 0x400000

// In una page directory ci sono 1024 entry ed ognuna descrive
// 4MB byte (1024 * 4MB = 4GB di memoria)
#define PAGE_DIRECTORY_ADDR_SPACE_SIZE 0x100000000

// Dimensione pagina standard (4kb)
#define PAGE_SIZE 0x1000

// La entry che permette la logica del paging ricorsivo
#define DIRECTORY_RECURSIVE_ENTRY 1023

// Definisce la costante higher half
#define HIGHER_HALF_PAGING 0xC0000000

// Definisce l'indirizzo degli ultimi 4MB
#define LAST_4MB_ADDRESS_SPACE 0xFFC00000

// Ultima page che é mappata alla page dir
#define KERNEL_PAGE_DIR 0xFFFFF000

// Struttura per page_table
struct page_table
{
	pte entries[PAGES_PER_TABLE];
};

// Struttura per page_directory
struct page_directory
{
	pde entries[TABLES_PER_DIR];
};

// Funzioni per la vmm
bool vmm_map(virtual_addr addr, physical_addr phys);
bool vmm_unmap(virtual_addr virt);
bool vmm_alloc_page(pte *entry);
bool vmm_free_page(pte *entry);
void *vmm_get_table(uint32_t entry_num);
bool vmm_switch_pdirectory(struct page_directory *dir);
struct page_directory *vmm_get_directory();
void vmm_initialize();

// Funzioni utili per gestire il paging
extern uint32_t read_cr0();
extern void set_cr0(uint32_t value);
extern uint32_t read_cr3();
extern void set_cr3(uint32_t value);
extern void vmm_flush_tlb_entry(virtual_addr addr);

#endif