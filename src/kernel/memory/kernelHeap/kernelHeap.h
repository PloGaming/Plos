#ifndef KERNELHEAP_H
#define KERNELHEAP_H

#include <stdbool.h>

#define KERNEL_HEAP_START_ADDR 0xD0000000

bool kheap_init();

#endif