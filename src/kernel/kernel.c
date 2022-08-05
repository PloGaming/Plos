#include <kernel.h>
#include <terminal/terminal.h>
#include <interrupts/interrupt.h>
#include <interrupts/isr.h>
#include <io/io.h>
#include <io/irq.h>
#include <io/io.h>
#include <strings/string.h>
#include <interrupts/syscalls.h>
#include <prekernel/prekernel.h>
#include <memory/paging/vmm.h>
#include <memory/kernelHeap/kernelHeap.h>
#include <devices/keyboard.h>
#include <config.h>

void kmain(multiboot_info_t *boot_info)
{

    // Inizializzazione dell'IDT (Interrupt descriptor table)
    idt_table_init();

    // Inizizializzazione degli ISR (Interrupt service routine) : Interrupt software
    ISR_Initialize();

    // Inizializzazione degli IRQ (Interrupt request) : Interrupt hardware
    IRQ_Initialize();

    // Aggiunta degli interrupt
    register_syscalls();

    // Abilita gli interrupt
    EnableInterrupts();

    // Inizializza l'heap del kernel
    if (!kheap_init())
        kernelPanic("Errore durante l'inizializzazione dell'heap kernel\n");

    // Inizializza la tastiera
    kkybrd_install();

    print_ascii_art();

    // Ricevi comandi
    run_shell(boot_info);
}

// Funzione Panic del kernel
void kernelPanic(char *message)
{
    printf(message);
    DisableInterrupts();
    while (1)
        ;
}

// Esegue una shell che legge un comando e lo esegue
// ripete tutto cio fino a che l'utente non immette il comando "exit"
void run_shell(multiboot_info_t *boot_info)
{
    // Definiamo un command buffer
    char cmd_buf[MAX_CMD_LENGTH];

    while (1)
    {
        // Otteniamo il comando
        get_cmd(cmd_buf, MAX_CMD_LENGTH);

        // Eseguiamo il comando
        if (run_cmd(cmd_buf, boot_info))
        {
            printf("\n\nUscita dal kernel...\n");

            // Se il comando è exit, esci dal kernel
            break;
        }
    }
}

// Esegue un comando, se il comando è exit ritorna true
// altrimenti ritorna falso
bool run_cmd(char *cmd, multiboot_info_t *boot_info)
{
    // Il comando "exit" esce dal kernel
    if (!strcmp(cmd, "exit"))
        return true;

    // il comando "print mem info" mostra delle informazioni sulla memoria
    if (!strcmp(cmd, "print mem info"))
    {
        printf("\n");
        print_system_information(boot_info);
    }

    // il comando "print mem layout" mostra il layout fisico della memoria
    if (!strcmp(cmd, "print mem layout"))
    {
        printf("\n");
        print_memory_map(boot_info);
    }

    // "Pulisce" lo schermo
    if (!strcmp(cmd, "clear"))
    {
        terminal_init();
    }

    // Mostra il menu delle opzioni
    if (!strcmp(cmd, "help"))
    {
        printf("\n");
        print_help();
    }

    return false;
}

void print_ascii_art()
{
    char *ascii =
        "________  ___       ________  ________         \n"
        "|\\   __  \\|\\  \\     |\\   __  \\|\\   ____\\       \n"
        "\\ \\  \\|\\  \\ \\  \\    \\ \\  \\|\\  \\ \\  \\___|_      \n"
        " \\ \\   ____\\ \\  \\    \\ \\  \\ \\  \\ \\_____  \\     \n"
        "  \\ \\  \\___|\\ \\  \\____\\ \\  \\ \\  \\|____|\\  \\    \n"
        "   \\ \\__\\    \\ \\_______\\ \\_______\\____\\_\\  \\   \n"
        "    \\|__|     \\|_______|\\|_______|\\_________\\  \n"
        "                                  \\|_________| \n";

    printf(ascii);
}

void print_help()
{
    printf("exit - Esce dal sistema operativo\n");
    printf("print mem info - Mostra delle informazioni sulla memoria\n");
    printf("print mem layout - Mostra il layout della memoria fisica\n");
    printf("clear - pulisce lo schermo\n");
    printf("help - mostra questo menu qua\n");
}