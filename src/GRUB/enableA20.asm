; Abilita il 21-esimo bit di ogni indirizzo di memoria
; Molto importante dato che senza questo non potremmo accedere a tutto
enable_A20Line:
    pusha
    in al, 0x92
    or al, 2
    out 0x92, al
    popa
    ret