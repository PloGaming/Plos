#include <memory/paging/pageDirectory.h>

void pde_add_flags(pde *entry, uint32_t flags)
{
	*entry |= flags;
}

void pde_clear_flags(pde *entry, uint32_t flags)
{
	*entry &= ~flags;
}

void pde_set_addr(pde *entry, uint32_t addr)
{
	*entry = (*entry & ~x86_PDE_PTE_BASE_ADDR) | addr;
}