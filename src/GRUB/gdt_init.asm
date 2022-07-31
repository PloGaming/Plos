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