[BITS 32]

section .asm

KERNEL_DATA_SEGMENT equ 0x10

extern ISR_Master_Handler
global isr_common

; Macro che definisce tutti gli isr senza un codice errore
%macro ISR_NOERRORCODE 1
global ISR%1:
ISR%1:
    push 0 ; Codice errore falso
    push %1 ; Codice interrupt
    jmp isr_common
%endmacro

; Macro che definisce tutti gli isr con un codice errore
%macro ISR_ERRORCODE 1
global ISR%1:
ISR%1:
    ; Codice errore inserito dalla CPU 
    push %1 ; Codice interrupt
    jmp isr_common
%endmacro

; Tutti gli isr
%include "src/kernel/interrupts/long_list.asm"

; Tutti gli isr passano da qua per inizializzarsi e per poi passare in c
isr_common:
    pusha ; Salva tutti i registri

    mov eax, 0
    mov ax, ds ; Salviamo il ds
    push eax

    mov ax, KERNEL_DATA_SEGMENT
    mov ds, ax ; modifichiamo tutti i segmenti 
    mov es, ax ; a quello del kernel
    mov fs, ax
    mov gs, ax

    push esp ; passiamo un puntatore allo stack del c

    ; Codice c che gestisce gli interrupt
    call ISR_Master_Handler
    
    add esp, 4
    pop eax

    mov ds, ax 
    mov es, ax 
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8 ; Rimuove l'interrupt number e anche l'error code
    iret