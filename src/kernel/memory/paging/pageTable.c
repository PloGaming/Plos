#include <memory/paging/pageTable.h>

void pte_add_flags(pte *entry, uint32_t flags)
{
	*entry |= flags;
}

void pte_clear_flags(pte *entry, uint32_t flags)
{
	*entry &= ~flags;
}

void pte_set_addr(pte *entry, uint32_t addr)
{
	*entry = (*entry & ~x86_PTE_FRAME) | addr;
}