#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Parametri page Table Entries (PTE)
#define x86_PTE_PRESENT 1
#define x86_PTE_WRITABLE 2
#define x86_PTE_USER 4
#define x86_PTE_NON_CACHE 0x10
#define x86_PTE_FRAME 0x7FFFF000

// Typedef per le Page Table Entries
typedef uint32_t pte;

// Funzioni per modificare un pte
void pte_add_flags(pte *entry, uint32_t flags);
void pte_clear_flags(pte *entry, uint32_t flags);
void pte_set_addr(pte *entry, uint32_t addr);
#endif