[BITS 32]

section .asm

global insbyte
global outbyte
global EnableInterrupts
global DisableInterrupts

;extern unsigned char input(unsigned short port)
insbyte:
    push ebp
    mov ebp, esp

    mov dx, [esp + 0x8]
    xor eax, eax

    in al, dx

    leave
    ret


outbyte:
    push ebp
    mov ebp, esp

    mov dx, [esp + 0x8]
    mov al, [esp + 0xC]
    out dx, al

    out dx, al

    leave
    ret

EnableInterrupts:
    sti
    ret

DisableInterrupts:
    cli
    ret