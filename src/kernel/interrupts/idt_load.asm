[BITS 32]
section .asm

global idt_table_load 

idt_table_load:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 0x8]
    lidt [eax]

    leave
    ret
