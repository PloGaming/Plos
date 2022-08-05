#ifndef KERNELHEAP_H
#define KERNELHEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <strings/string.h>

#define KERNEL_HEAP_START_ADDR 0xD0000000
#define KERNEl_HEAP_END_ADDR 0xE0000000

#define HAS_N 0b10000000
#define IS_FIRST 0b01000000
#define IS_FREE 0b00100000
#define HEAP_END 0b00010000

#define HEAP_SIZE 0x10000000   // 256 MB
#define HEAP_BLOCK_SIZE 0x1000 // 4096 byte

struct heap_table
{
	virtual_addr *entries;
	uint32_t total_entries;
};

struct heap
{
	struct heap_table *table;
	virtual_addr *heap_start;
};

bool kheap_init();
bool kheap_enlarge(size_t n);
void *kmalloc(int n);
void *kmalloc_debug(int n);
int get_blocks(size_t n);

#endif