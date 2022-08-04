[BITS 32]
global _start ; Esporta il simbolo in questa maniera possiamo compilare senza errori

extern kernel_start
extern kernel_end

MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

CODE_SEG equ 0x08
DATA_SEG equ 0x10

[section .multiboot.data]
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM


[section .bootstrap_stack]

stack_bottom:
    resb 16384 ; 16 KiB
stack_top:


[section .bss]
align 4096
    boot_page_directory:
        resb 4096 ; 4Kb
    boot_page_table1:
        resb 4096 ; 4kb
    ; Se il kernel > 4 MB è necessario aggiungere altre page_tables


[section .multiboot.text]

; Da questo momento ci troviamo in modalita protetta 32 bit,
; gli interrupts sono disabilitati, il paging e disabilitato
; e lo stato della CPU é definito nello standard multiboot
_start:

    ; Setup dello stack
	mov esp, stack_top
	mov ebp, esp

; GRUB non garantisce l'effettivo caricamento della GDT percio dobbiamo farlo noi
.loadGDT:
    
    lgdt [gdt_descriptor] ; gdtr viene impostato
	jmp CODE_SEG:resume ; long jump necessario per ricaricare cs

resume:

    ; Setup di tutti i registri segmento riguardanti i dati
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

; Abilitazione A20 line tramite il metodo fast gate 
.enableA20:

    in al, 0x92
    or al, 2
    out 0x92, al

; Abilitazione paging per kernel higher half (3GB 0xC0000000)
.paging:

    ; Indirizzo fisico della page_table (Sottraiamo 0xC0000000 a causa del linker)
    mov edi, (boot_page_table1 - 0xC0000000)
    ; Lo 0 indica il primo indirizzo da mappare (esi è una pte, stiamo impostando il frame)
    mov esi, 0
    ; Vogliamo eseguire il map delle prime 1023 pagine (la 1024 è il VGA buffer)
    mov ecx, 1023

one:
    cmp esi, kernel_start
    jl two
    cmp esi, (kernel_end - 0xC0000000)
    jge three

    ; edx conterra l'indirizzo di una pagina del kernel
    mov edx, esi
    ; la pagina é impostata come presente e scrivibile
    or edx, 0x003
    ; Spostiamo nella page table la nostra page table entry
    mov [edi], edx

; Aggiungeremo una nuova page table entry fino a quando non 
; abbiamo mappato tutto il kernel
two:
    ; la grandezza di una pagina è 4096 bytes
    add esi, 4096
    ; La dimensione di una pte è 4 bytes
    add edi, 4
    
    ; Al termine del loop avremo una page_table contenente
    ; tutte le pagine del kernel (il kernel inizia a 0x00100000 (1MB))
    loop one

; Ora eseguiamo il map del VGA buffer a 0xC03FF000
three:
    ; eax rappresenta una pte che punta al VGA buffer
    ; ma presente e scrivibile
    mov eax, 0x000B8003

    ;modifichiamo la 1024esima table entry con l'indirizzo del VGA buffer
    mov [boot_page_table1 - 0xC0000000 + 1023 * 4],  eax
    
    ; Stiamo facendo un identity map del kernel (da 0 fino a 4 MB) in questa 
    ; maniera le istruzioni verranno eseguite senza problemi (senza page fault)
    ; dato che sono come gli indirizzi fisici. Inoltre abbiamo mappato il kernel 
    ; anche ad 0xC0000000 in questa maniera il kernel sara higher half
    ; (non ci servira piu la "lowe half" entry percio la elimineremo quando non sara piu necessaria)

    ; eax sarà una pde, conterrà l'indirizzo fisico della page table e gli attributi (0x003)
    mov eax, (boot_page_table1 - 0xC0000000 + 0x003)

    ; Lower half table impostata
    mov [boot_page_directory - 0xC0000000 + 0], eax

    ; Higher half table impostata
    mov [boot_page_directory - 0xC0000000 + 768 * 4], eax

    ; eax conterra l'indirizzo della page directory
    mov eax, (boot_page_directory - 0xC0000000 + 0x003)

    ; La 1023esima entry deve puntare alla page directory per recursive paging 
    mov [boot_page_directory - 0xC0000000 + 1023 * 4], eax

    ; Impostiamo CR3 all'indirizzo della boot_page_directory
    mov eax, (boot_page_directory - 0xC0000000)
    mov cr3, eax 

    ; Abilitiamo il paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Entriamo nell'Higher half kernel
    lea eax, [four]
    jmp eax

    cli
sto:
    hlt
    jmp sto
 
; Questo è l'inizio della gdt table
gdt_table_start:

; La prima entry della gdt deve essere nulla
gdt_null_entry:
    dd 0x0
    dd 0x0

; La entry per il segmento code del kernel (0x0 - 0xxfffffff)
gdt_code_entry:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

; La entry per il segmento data del kernel (0x0 - 0xxfffffff)
gdt_data_entry:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

; fine gdt table
gdt_table_finish:

; il registro gdtr deve puntare a questo
gdt_descriptor:
    dw gdt_table_finish - gdt_table_start - 1
    dd gdt_table_start

; indice per il segmento Code and Data
CODE_SEG equ gdt_code_entry - gdt_table_start
DATA_SEG equ gdt_data_entry - gdt_table_start

[section .text]

extern prekernel ; Importiamo il simbolo da file, questo é il nome del main del kernel
global BREAKPOINT

; A questo punto il paging è impostato correttamente
; Il nostro kernel si trova a 0xC0100000
four:

    mov eax, 0x00
    mov [boot_page_directory + 0], eax
    mov ecx, cr3
    mov cr3, ecx

; Una volta abilitato il paging dobbiamo ricaricare la gdt
.reload_gdt_page:

    lgdt [page_gdt_descriptor] ; gdtr viene impostato
	jmp PAGE_CODE_SEG:page_resume

page_resume:

     ; Setup di tutti i registri segmento riguardanti i dati
    mov ax, PAGE_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Aggiungiamo 0xC0000000 alla memory map di GRUB 
    ; perchè adesso stiamo usando paging e l'indirizzo é sbagliato
    add ebx, 0xC0000000
    push ebx 
    call prekernel

    ; Quando torniamo dal kernel non facciamo nulla
stop:
    hlt
    jmp stop

; Questo è l'inizio della gdt table
page_gdt_table_start:

; La prima entry della gdt deve essere nulla
page_gdt_null_entry:
    dd 0x0
    dd 0x0

; La entry per il segmento code del kernel (0x0 - 0xxfffffff)
page_gdt_code_entry:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

; La entry per il segmento data del kernel (0x0 - 0xxfffffff)
page_gdt_data_entry:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

; fine gdt table
page_gdt_table_finish:

; il registro gdtr deve puntare a questo
page_gdt_descriptor:
    dw page_gdt_table_finish - page_gdt_table_start - 1
    dd page_gdt_table_start

; indice per il segmento Code and Data
PAGE_CODE_SEG equ page_gdt_code_entry - page_gdt_table_start
PAGE_DATA_SEG equ page_gdt_data_entry - page_gdt_table_start

; Per evitare problemi di allineamento con il codice in c 
;(dato che questo file anche se .asm verra compilato vicino al codice c)
times 512 - ($ - $$) db 0