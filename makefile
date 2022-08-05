CROSS_PATH=$(HOME)/opt/cross/bin
CC=$(CROSS_PATH)/i686-elf-gcc
LD=$(CROSS_PATH)/i686-elf-ld

# Tutte queste flag servono per dire al compilatore di:
# 1) Non cercare il solito 'int main()'
# 2) Allineare ogni chiamata di funzione, label, loop, ecc...
# 3) Fornirci piu warnings
# 4) Trasformare i warnings in errori
# 5) Abilitare i simboli di debug (caricabili da kernelfull.o a partire da 0x0100000)
# 6) Disabilitare l'ottimizzazione del codice
CFLAGS=-g -ggdb -ffreestanding -falign-jumps -falign-functions -falign-labels \
	-falign-loops -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -Wno-unused-function -fno-builtin -Werror \
	-Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib \
	-nostartfiles -nodefaultlibs -Wall -O0

# Il path dove il nostro compilatore cerchera gli headers
INCLUDES=-I src/kernel

QEMU=qemu-system-i386

OBJS=bin/GRUB.asm.o bin/kernel.o bin/terminal.o bin/string.o bin/idt_load.asm.o bin/interrupt.o bin/memory.o \
	bin/isr.asm.o  bin/isr.o bin/isrs_gen.o bin/io.asm.o bin/io.o bin/pic.o bin/irq.o \
	bin/utility_functions.asm.o bin/pmm.o bin/pageDirectory.o bin/pageTable.o bin/vmm.o bin/syscalls.o \
	bin/prekernel.o bin/kernelHeap.o bin/getCr2.asm

all: bin/kernel linker.ld
	./iso.sh

bin/kernel: $(OBJS)
	$(LD) -g -relocatable $(OBJS) -o bin/kernelfull.o
	$(CC) $(CFLAGS) -T linker.ld -o $@ -ffreestanding -O0 -nostdlib bin/kernelfull.o

bin/GRUB.asm.o: src/GRUB/boot.asm
	nasm -f elf -g $< -o $@

bin/kernel.o: src/kernel/kernel.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/idt_load.asm.o: src/kernel/interrupts/idt_load.asm
	nasm -f elf -g $< -o $@

bin/isr.asm.o: src/kernel/interrupts/isr.asm
	nasm -f elf -g $< -o $@

bin/io.asm.o: src/kernel/io/io.asm
	nasm -f elf -g $< -o $@

bin/utility_functions.asm.o: src/kernel/memory/paging/utility_functions.asm
	nasm -f elf -g $< -o $@

bin/getCr2.asm: src/kernel/interrupts/getCr2.asm
	nasm -f elf -g $< -o $@

bin/terminal.o: src/kernel/terminal/terminal.c src/kernel/terminal/terminal.h
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/string.o: src/kernel/strings/string.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/interrupt.o: src/kernel/interrupts/interrupt.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/memory.o: src/kernel/memory/memory.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/isr.o: src/kernel/interrupts/isr.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/isrs_gen.o: src/kernel/interrupts/isrs_gen.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/io.o: src/kernel/io/io.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/pic.o: src/kernel/io/pic.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/irq.o: src/kernel/io/irq.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/pmm.o: src/kernel/memory/pmm/pmm.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/pageDirectory.o: src/kernel/memory/paging/pageDirectory.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/pageTable.o: src/kernel/memory/paging/pageTable.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/vmm.o: src/kernel/memory/paging/vmm.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/syscalls.o: src/kernel/interrupts/syscalls.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/prekernel.o: src/kernel/prekernel/prekernel.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

bin/kernelHeap.o: src/kernel/memory/kernelHeap/kernelHeap.c
	$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -c $< -o $@

run: all
	$(QEMU) -cdrom build/PlOS.iso

debug: all
	$(QEMU) -s -S -cdrom build/PlOS.iso

clean:
	rm -Rf $(OBJS)
	rm -Rf bin/kernel
	rm -Rf bin/kernelfull.o
	rm -Rf build/PlOS.iso

%.asm: ;