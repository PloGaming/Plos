#ifndef KERNELHEAP_H
#define KERNELHEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <strings/string.h>

#define KERNEL_HEAP_START_ADDR 0xD0000000
#define KERNEl_HEAP_END_ADDR 0xE0000000

bool kheap_init();
bool kheap_enlarge(size_t n);

#endif