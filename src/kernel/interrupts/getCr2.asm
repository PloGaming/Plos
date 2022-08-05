[BITS 32]

section .asm

global get_cr2

get_cr2:
	push ebp
	mov ebp, esp

	mov eax, cr2

	leave
	ret