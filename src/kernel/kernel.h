#ifndef KERNEL_H
#define KERNEL_H

#include <strings/string.h>
#include <io/io.h>
#include <../GRUB/multiboot.h>

void kmain();
void kernelPanic(char *message);
void print_system_information(multiboot_info_t *boot_info, uint8_t *kernel_start, uint8_t *kernel_end, size_t kernel_size, uint32_t memSize);
#endif