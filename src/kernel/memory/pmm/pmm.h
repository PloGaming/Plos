/* DEFINIZIONE:
The purpose of a physical memory manager is to split
up the computers physical address space into block-sized
chunks of memory and provide a method to allocate and release them

PMM = Physical memory manager
*/

#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <../GRUB/multiboot.h>

#define PMM_BLOCK_SIZE 4096 // 4Kb

void pmm_free_block(void *p);
void *pmm_alloc_block();
void pmm_deinit_kernel();
void pmm_init_available_regions(uint32_t _mmap_start, uint32_t _mmap_end);
void pmm_deinit_region(uint32_t base_addr, size_t size);
void pmm_init_region(uint32_t base_addr, size_t size);
void pmm_init(size_t memSize, uint32_t *bitmap);
int get_free_blocks();
int get_used_blocks();
void *get_memory_map_addr();
void *pmm_alloc_blocks(size_t size);

#endif