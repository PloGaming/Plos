#include <io/pic.h>
#include <io/io.h>

enum
{
    PIC_ICW1_ICW4 = 0x01,
    PIC_ICW1_SINGLE = 0x02,
    PIC_ICW1_INTERVAL4 = 0x04,
    PIC_ICW1_LEVEL = 0x08,
    PIC_ICW1_INITIALIZE = 0x10
} PIC_ICW1;

enum
{
    PIC_ICW4_8086 = 0x1,
    PIC_ICW4_AUTO_EOI = 0x2,
    PIC_ICW4_BUFFER_MASTER = 0x4,
    PIC_ICW4_BUFFER_SLAVE = 0x0,
    PIC_ICW4_BUFFERRED = 0x8,
    PIC_ICW4_SFNM = 0x10,
} PIC_ICW4;

void PIC_sendEOI(unsigned char irq)
{
    if (irq >= 8)
    {
        PIC_send_command(PIC_EOI, 1);
    }

    PIC_send_command(PIC_EOI, 0);
}

uint8_t PIC_read_data(uint8_t pic_Num)
{
    if (pic_Num > 1)
    {
        return 0;
    }

    uint8_t reg = pic_Num == 0 ? PIC1_DATA : PIC2_DATA;
    uint8_t data = insbyte(reg);
    io_wait();
    return data;
}

// Useremo questo solo per scrivere sul registro IMR per maskerare gli iqr che vogliamo
void PIC_send_data(uint8_t data, uint8_t pic_num)
{
    if (pic_num > 1)
    {
        return;
    }

    uint8_t reg = pic_num == 0 ? PIC1_DATA : PIC2_DATA;
    outbyte(reg, data);
    io_wait();
}

// Invia un comando al PIC master o slave
void PIC_send_command(uint8_t command, uint8_t pic_num)
{
    if (pic_num > 1)
    {
        return;
    }

    uint8_t addr = pic_num == 0 ? PIC1_COMMAND : PIC2_COMMAND;
    outbyte(addr, command);
    io_wait();
}

void PIC_remap(int offset1, int offset2)
{
    // Inizio inizializzazione Pic inviando icw1
    PIC_send_command(PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE, 0); // ICW1 command; The bit number 4 is set because we have to initialize PIC
    PIC_send_command(PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE, 1); // Bit number 8 says it's mandatory sending ICW4 command

    // Invio initialization control word 2, imposta l'indirizzo base degli irqs
    PIC_send_data(offset1, 0);
    PIC_send_data(offset2, 1);

    // Invio initialization control word 3, imposta la connessione tra il master PIC e lo slave PIC
    PIC_send_data(0x4, 0);
    PIC_send_data(0x2, 1);

    // Invio initialization control word 4, imposta la modalita x86
    PIC_send_data(PIC_ICW4_8086, 0);
    PIC_send_data(PIC_ICW4_8086, 1);

    // Cancellare i valori nei registri del PIC
    PIC_send_data(0, 0);
    PIC_send_data(0, 1);
}

void PIC_Mask(int irq)
{
    uint8_t mask;

    if (irq < 8)
    {
        mask = PIC_read_data(0);
        PIC_send_data(mask | (1 << irq), 0);
    }
    else
    {
        irq -= 8;
        mask = PIC_read_data(1);
        PIC_send_data(mask | (1 << irq), 1);
    }
}

void PIC_Unmask(int irq)
{
    uint8_t mask;

    if (irq < 8)
    {
        mask = PIC_read_data(0);
        PIC_send_data(mask & ~(1 << irq), 0);
    }
    else
    {
        irq -= 8;
        mask = PIC_read_data(1);
        PIC_send_data(mask & ~(1 << irq), 1);
    }
}

void PIC_Disable(int irq)
{
    PIC_send_data(0xff, 0);
    PIC_send_data(0xff, 1);
}

uint16_t PIC_GetIRQRequestRegister()
{
    PIC_send_command(0x0a, 0);
    PIC_send_command(0x0a, 1);
    return ((insbyte(PIC2_COMMAND) << 8) | insbyte(PIC1_COMMAND));
}

uint16_t PIC_GetIRQServiceRegister()
{
    PIC_send_command(0x0b, 0);
    PIC_send_command(0x0b, 1);
    return ((insbyte(PIC2_COMMAND) << 8) | insbyte(PIC1_COMMAND));
}

void PIC_init()
{
    PIC_remap(PIC1_OFFSET, PIC2_OFFSET);
}