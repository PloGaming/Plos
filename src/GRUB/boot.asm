MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

CODE_SEG equ 0x08
DATA_SEG equ 0x10

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .bss
align 16
stack_bottom:
	resb 16384 ; 16 KiB
stack_top:

[BITS 32]
section .text

global _start ; Esporta il simbolo in questa maniera possiamo compilare senza errori
extern prekernel ; Importiamo il simbolo da file, questo é il nome del main del kernel

; Da questo momento ci troviamo in modalita protetta 32 bit,
; gli interrupts sono disabilitati, il paging e disabilitato
; e lo stato della CPU e definito nello standard multiboot
_start:

	; Setup dello stack
	mov esp, stack_top
	mov ebp, esp

; GRUB non garantisce l'effettivo caricamento della GDT percio dobbiamo farlo noi
.loadGDT:
    
    lgdt [gdt_descriptor] ; gdtr viene impostato
	jmp CODE_SEG:_start.resume

.resume:

    ; Setup di tutti i registri segmento riguardanti i dati
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Impostare EAX come specificato dallo standard multiboot GRUB 
    mov eax, 0x2BADB002

    ; Abilitiamo la A20 line
    call enable_A20Line

    ; Entriamo nel kernel
	push ebx ; Contiene l'indirizzo fisica di una struttura dati multiboot
    call prekernel

    ; Quando torniamo dal kernel non facciamo nulla
    jmp $

.end:

%include "src/GRUB/gdt_init.asm"
%include "src/GRUB/enableA20.asm"

; Per evitare problemi di allineamento con il codice in c 
;(dato che questo file anche se .asm verra compilato vicino al codice c)
times 512 - ($ - $$) db 0