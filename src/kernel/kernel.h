#ifndef KERNEL_H
#define KERNEL_H

#include <strings/string.h>
#include <io/io.h>
#include <../GRUB/multiboot.h>

void kmain();
void kernelPanic(char *message);

#endif