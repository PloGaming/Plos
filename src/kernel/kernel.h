#ifndef KERNEL_H
#define KERNEL_H

#include <strings/string.h>
#include <io/io.h>
#include <../GRUB/multiboot.h>

void kmain(multiboot_info_t *boot_info);
void kernelPanic(char *message);

void run_shell(multiboot_info_t *boot_info);
bool run_cmd(char *cmd, multiboot_info_t *boot_info);
void print_ascii_art();
void print_help();

#endif