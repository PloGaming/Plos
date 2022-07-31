#ifndef PAGEDIRECTORY_H
#define PAGEDIRECTORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Parametri Page Directory Entries (PDE)
#define x86_PDE_PRESENT 1
#define x86_PDE_WRITABLE 2
#define x86_PDE_USER 4
#define x86_PDE_NON_CACHE 0x10
#define x86_PDE_PTE_BASE_ADDR 0x7FFFF000

// Typedef per le Page Directory Entries
typedef uint32_t pde;

// Funzioni per modificare un PDE
void pde_add_flags(pde *entry, uint32_t flags);
void pde_clear_flags(pde *entry, uint32_t flags);
void pde_set_addr(pde *entry, uint32_t addr);

#endif