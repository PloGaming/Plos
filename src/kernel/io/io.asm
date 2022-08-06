[BITS 32]

section .asm

global insbyte
global outbyte
global insword
global outword
global EnableInterrupts
global DisableInterrupts

insbyte:
    push ebp
    mov ebp, esp

    mov dx, [esp + 0x8]
    xor eax, eax

    in al, dx

    leave
    ret

insword:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8]
    in ax, dx

    leave
    ret

outbyte:
    push ebp
    mov ebp, esp

    mov dx, [esp + 0x8]
    mov al, [esp + 0xC]

    out dx, al

    leave
    ret

outword:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 12]
    mov edx, [ebp + 8]

    out dx, ax

    leave
    ret

EnableInterrupts:
    sti
    ret

DisableInterrupts:
    cli
    ret