[BITS 32]

section .asm

global read_cr0
global set_cr0

global read_cr3
global set_cr3

global vmm_flush_tlb_entry

read_cr0:
	mov eax, cr0
	ret

set_cr0:
	push ebp
	mov ebp, esp

	mov eax, [ebp + 0x8]
	mov cr0, eax

	leave
	ret

read_cr3:
	mov eax, cr3
	ret

set_cr3:
	push ebp
	mov ebp, esp

	mov eax, [ebp + 0x8]
	mov cr3, eax

	leave
	ret

vmm_flush_tlb_entry:
	push ebp
	mov ebp, esp

	cli
	invlpg [ebp + 0x8]
	sti
	leave
	ret
