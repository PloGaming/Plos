#ifndef PREKERNEL_H
#define PREKERNEL_H

#include <../GRUB/multiboot.h>
#include <stddef.h>
#include <stdint.h>

void print_system_information(multiboot_info_t *boot_info, uint8_t *kernel_start, uint8_t *kernel_end, size_t kernel_size, uint32_t memSize);

#endif