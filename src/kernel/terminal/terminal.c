#include <terminal/terminal.h>
#include <strings/string.h>
#include <kernel.h>
#include <stdint.h>
#include <stdbool.h>

struct terminal_position terminal;

// Inserisce un carattere con un determinato attributo ad una data posizione
void terminal_putchar(int position, char character, uint8_t attribute)
{
    uint16_t *charAddr = (uint16_t *)VGA_START_ADDRESS + position;
    *charAddr = character | (attribute << 8);
}

// Restituisce (solo il carattere) ad una data posizione
char terminal_getChar(int position)
{
    uint16_t *charAddr = (uint16_t *)VGA_START_ADDRESS + position;
    return (*charAddr & 0x0000ffff);
}

void catch (int number)
{
    return;
}

void terminal_scroll()
{
    // Elimina la prima riga
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putchar(i, ' ', WHITE_ON_BLUE);
    }

    char character;
    // portare indietro dalla 2 riga una fino all'ultima
    for (int i = VGA_WIDTH; i < VGA_HEIGTH * VGA_WIDTH; i++)
    {
        character = terminal_getChar(i);
        terminal_putchar(i - VGA_WIDTH, character, WHITE_ON_BLUE);
    }

    // Pulisce l'ultima riga
    for (int i = VGA_WIDTH * (VGA_HEIGTH - 1); i < VGA_WIDTH * VGA_HEIGTH; i++)
    {
        terminal_putchar(i, ' ', WHITE_ON_BLUE);
    }

    terminal.y = 24;
    terminal.x = 0;
}

bool terminal_checkScroll()
{
    if (terminal.y == VGA_HEIGTH)
    {
        return true;
    }
    return false;
}

// Il flow va su una nuova riga
void terminal_add_newLine()
{
    if (terminal_checkScroll())
    {
        terminal_scroll();
        return;
    }
    terminal.x = 0;
    terminal.y++;
}

// Inserisce una stringa tenendo conto della posizione delle precedenti
void terminal_putstring(char *string)
{
    int length = strlen(string);
    int position;

    for (int i = 0; i < length; i++)
    {
        if (*(string + i) == '\n')
        {
            terminal_add_newLine();
            continue;
        }

        if (terminal_checkScroll())
        {
            terminal_scroll();
        }

        if (terminal.x == VGA_WIDTH)
        {
            terminal_add_newLine();
        }
    do_stuff:
        position = (terminal.y * VGA_WIDTH) + terminal.x++;
        catch (position);
        terminal_putchar(position, *(string + i), WHITE_ON_BLUE);
        catch (position);
    }
}

void print_char(char character)
{
    char fake_string[2] = {character, '\0'};
    terminal_putstring(fake_string);
}

void print(char *string)
{
    terminal_putstring(string);
}

// Pulisce lo schermo
void clear_screen()
{
    for (int i = 0; i < VGA_HEIGTH * VGA_WIDTH; i++)
    {
        terminal_putchar(i, ' ', WHITE_ON_BLUE);
    }
}

// Funzione che inizializza il terminale
void terminal_init()
{
    clear_screen();
    terminal.x = 0;
    terminal.y = 0;
}