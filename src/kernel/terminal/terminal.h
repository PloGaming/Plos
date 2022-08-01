#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

#define VGA_WIDTH 80
#define VGA_HEIGTH 25
#define WHITE_ON_BLUE 0x1f

#define VGA_START_ADDRESS 0xC03FF000

struct terminal_position
{
    int x;
    int y;
};

void print(char *string);
char terminal_getChar(int position);
void terminal_scroll();
bool terminal_checkScroll();
void terminal_add_newLine();
void terminal_putchar(int position, char character, uint8_t attribute);
void terminal_putstring(char *string);
void terminal_clear_screen();
void terminal_init();
void print_char(char character);

#endif