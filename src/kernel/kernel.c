#include <kernel.h>
#include <terminal/terminal.h>
#include <interrupts/interrupt.h>
#include <interrupts/isr.h>
#include <io/io.h>
#include <io/irq.h>
#include <io/io.h>
#include <strings/string.h>
#include <interrupts/handlers.h>
#include <prekernel/prekernel.h>
#include <memory/paging/vmm.h>
#include <memory/kernelHeap/kernelHeap.h>
#include <devices/keyboard.h>
#include <config.h>
#include <devices/pit.h>
#include <disk/ata.h>
#include <devices/mouse.h>

static struct ATA_Device Master1;
static struct ATA_Device Slave1;
static struct ATA_Device Master2;
static struct ATA_Device Slave2;

void kmain(multiboot_info_t *boot_info)
{
    // Inizializzazione dell'IDT (Interrupt descriptor table)
    idt_table_init();

    // Inizizializzazione degli ISR (Interrupt service routine) : Interrupt software
    ISR_Initialize();

    // Inizializzazione degli IRQ (Interrupt request) : Interrupt hardware
    IRQ_Initialize();

    // Aggiunta degli interrupt
    register_handlers();

    // Abilita gli interrupt
    EnableInterrupts();

    // Inizializza l'heap del kernel
    if (!kheap_init())
        kernelPanic("Errore durante l'inizializzazione dell'heap kernel\n");

    // Inizializza la tastiera
    kkybrd_install();

    // Inizializza il PIT
    pit_start_counter(100, I86_PIT_OCW_COUNTER_0, I86_PIT_OCW_MODE_SQUAREWAVEGEN);

    // Inizializza il mouse
    // mouse_initialize();

    // Inizializziamo i primi 4 dispositivi ATA (master e slave)
    Initialize_ATA_Device(&Master1, 0x1F0, true);
    Initialize_ATA_Device(&Slave1, 0x1F0, false);

    Identify(&Master1);

    Initialize_ATA_Device(&Master2, 0x170, true);
    Initialize_ATA_Device(&Slave2, 0x170, false);
    // TODO: Inizializzare altri ATA device

    // Mostra un messaggio di avvio
    print_ascii_art();

    // Inizia ad eseguire comandi
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

    if (!strcmp(cmd, "ticks"))
    {
        printf("\nNumero di tick passati %d\n", get_tick_count());
    }

    // Mostra il menu delle opzioni
    if (!strcmp(cmd, "help"))
    {
        printf("\n");
        print_help();
    }

    // Mostra sullo schermo i primi 10 byte di dati nell'hard disk principale
    if (!strcmp(cmd, "test"))
    {
        // TODO
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

// Mostra il messaggio che spiega i comandi
void print_help()
{
    printf("exit - Esce dal sistema operativo\n");
    printf("print mem info - Mostra delle informazioni sulla memoria\n");
    printf("print mem layout - Mostra il layout della memoria fisica\n");
    printf("clear - pulisce lo schermo\n");
    printf("ticks - mostra il numero corrente di tick passati\n");
    printf("test - mostra i primi 10 byte dell'hard disk principale\n");
    printf("help - mostra questo menu qua\n");
}

// Funzione che mette aspetta ms millisecondi
void sleep(int ms)
{
    DisableInterrupts();
    int temp = get_tick_count() + ms;
    while (get_tick_count() < temp)
        ;
    EnableInterrupts();
}