#include <terminal/terminal.h>
#include <strings/string.h>
#include <kernel.h>
#include <stdint.h>
#include <stdbool.h>
#include <devices/keyboard.h>

struct terminal_position terminal;

uint8_t color_terminal = WHITE_ON_BLUE;

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
        terminal_putchar(i, ' ', color_terminal);
    }

    char character;
    // portare indietro dalla 2 riga una fino all'ultima
    for (int i = VGA_WIDTH; i < VGA_HEIGTH * VGA_WIDTH; i++)
    {
        character = terminal_getChar(i);
        terminal_putchar(i - VGA_WIDTH, character, color_terminal);
    }

    // Pulisce l'ultima riga
    for (int i = VGA_WIDTH * (VGA_HEIGTH - 1); i < VGA_WIDTH * VGA_HEIGTH; i++)
    {
        terminal_putchar(i, ' ', color_terminal);
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

        position = (terminal.y * VGA_WIDTH) + terminal.x++;
        catch (position);
        terminal_putchar(position, *(string + i), color_terminal);
        catch (position);
    }

    update_cursor(terminal.x, terminal.y);
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
        terminal_putchar(i, ' ', color_terminal);
    }
}

// Funzione che inizializza il terminale
void terminal_init()
{
    clear_screen();
    terminal.x = 0;
    terminal.y = 0;
    enable_cursor(terminal.x, terminal.y);
}

// Abilita il cursore
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outbyte(0x3D4, 0x0A);
    outbyte(0x3D5, (insbyte(0x3D5) & 0xC0) | cursor_start);

    outbyte(0x3D4, 0x0B);
    outbyte(0x3D5, (insbyte(0x3D5) & 0xE0) | cursor_end);
}

// Sposta il cursore
void update_cursor(int x, int y)
{
    uint16_t pos = ++y * VGA_WIDTH + x;

    outbyte(0x3D4, 0x0F);
    outbyte(0x3D5, (uint8_t)(pos & 0xFF));
    outbyte(0x3D4, 0x0E);
    outbyte(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void cmd()
{
    printf("\nPlos> ");
    update_cursor(terminal.x, terminal.y);
}

// Ottiene un comando di max lenght n e lo salva in buf
void get_cmd(char *buf, int n)
{
    // Mostra il messaggio iniziale
    cmd();

    enum KEYCODE key = KEY_UNKNOWN;
    bool bufChar;

    int i;

    for (i = 0; i < n;)
    {
        // Variabile che indica se un carattere deve
        // essere salvato oppure no
        bufChar = true;

        // Ottieni il prossimo caratere
        key = getch();

        // Se la key è il pulsante invio esci dal ciclo
        if (key == KEY_RETURN)
            break;

        if (key == KEY_BACKSPACE)
        {
            // Non salviamo il carattere
            bufChar = false;

            // Dobbiamo tornare indietro di un carattere
            if (i > 0)
            {
                // Imposta le coordinate del cursore 1 indietro
                if (terminal.x > 0)
                {
                    terminal.x--;
                    update_cursor(terminal.x, terminal.y);
                }

                // Rimuoviamo il carattere dal display
                print_char(' ');
                terminal.x--;
                update_cursor(terminal.x, terminal.y);

                // Andiamo indietro un carattere nel buffer dei caratteri
                i--;
            }
        }

        switch (key)
        {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
        case KEY_CAPSLOCK:
        case KEY_LCTRL:
        case KEY_RCTRL:
        case KEY_LALT:
        case KEY_RALT:
        case KEY_ESCAPE:
            // Non vogliamo mostrare sullo schermo questi caratteri
            bufChar = false;

        default:
            break;
        }

        if (bufChar)
        {
            // Converte la key in ASCII e lo mette nel buffer
            char c = kkybrd_key_to_ascii(key);

            // Controllo che sia valido
            if (c != 0)
            {
                print_char(c);
                buf[i++] = c;
            }
        }
    }

    // Null terminiamo la stringa
    buf[i] = '\0';
}