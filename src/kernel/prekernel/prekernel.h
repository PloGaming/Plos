#ifndef PREKERNEL_H
#define PREKERNEL_H

#include <../GRUB/multiboot.h>
#include <stddef.h>
#include <stdint.h>

void print_system_information(multiboot_info_t *boot_info);
void print_memory_map(multiboot_info_t *boot_info);

#endif