/*
    Linker script che definisce in che indirizzi di memoria mettere ogni sezione 
    NB si occupa solo dei file riguardanti il kernel NON del bootloader
*/

ENTRY(_start) 

SECTIONS
{
    /*
        Tutto il codice di tutti i file .c e dell'UNICO file .asm kernel.asm verra
        messo ad 1MB
    */
    . = 1M;
    kernel_start = .;
    .text : ALIGN(4096)
    {
        *(.multiboot)
        *(.text)
    }

    /*
        static costants read-only data section, inserita subito dopo il codice del kernel
    */
    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    /*
        data section, contiene variabile inizializzate statiche (es variabili globali o locali statiche)
    */
    .data : ALIGN(4096)
    {
        *(.data)
    }

    /*
        bss section, contiene variabili non inizializzate statiche (o globali)
    */
    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }

    /*
        Tutti i file .asm devono avere section .asm all'inizio in questa maniera verranno inseriti come ultimi
        elementi all'interno della memoria.
        Questo per evitare che il codice asm si scontri con il codice c, dato che il .c richiede che sia tutto allineato
        perfettamente per funzionare mentre asm no
    */
    .asm : ALIGN(4096)
    {
        *(.asm)
    }
    kernel_end = .;
}

/*
    Allineamo tutte le sezioni a 4096 byte in modo che il codice .c non dia problemi
*/