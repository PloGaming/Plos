/*
    Linker script che definisce in che indirizzi di memoria mettere ogni sezione 
    NB si occupa solo dei file riguardanti il kernel NON del bootloader
*/

ENTRY(_start) 

SECTIONS
{
    . = 1M;
    
    .multiboot.data : {
            *(.multiboot.data)
    }

    .multiboot.text : {
           *(.multiboot.text)
    }

    . += 0xC0000000;
     
    .text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
    {
        kernel_start = .;
        *(.text)
    }

    /*
        static costants read-only data section, inserita subito dopo il codice del kernel
    */
    .rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    /*
        data section, contiene variabile inizializzate statiche (es variabili globali o locali statiche)
    */
    .data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data)
    }

    
    /*
        Tutti i file .asm devono avere section .asm all'inizio in questa maniera verranno inseriti come ultimi
        elementi all'interno della memoria.
        Questo per evitare che il codice asm si scontri con il codice c, dato che il .c richiede che sia tutto allineato
        perfettamente per funzionare mentre asm no
    */

    .asm ALIGN(4K) : AT (ADDR( .asm) - 0xC0000000)
    {
        *(.asm)
    }

    /*
        bss section, contiene variabili non inizializzate statiche (o globali)
    */
    
    .bss ALIGN(4K) : AT (ADDR( .bss) - 0xC0000000)
    {
        *(COMMON)
        page_structures = .;
        *(.bss)
        *(.bootstrap_stack)
    }

    
    
    kernel_end = .;
}

/*
    Allineamo tutte le sezioni a 4096 byte in modo che il codice .c non dia problemi
*/