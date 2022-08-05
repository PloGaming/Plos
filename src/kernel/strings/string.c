#include <strings/string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <terminal/terminal.h>

// Data una stringa di lunghezza variabile (null-terminata), ritorna la sua lunghezza
int strlen(char *string)
{
    char *str2 = string;
    while (*str2 != '\0')
    {
        str2++;
    }

    return str2 - string;
}

static void print_uint_to_string(unsigned long number, unsigned long base)
{
    /* error checking */
    if (base > 16)
    {
        print("printf base cannot be greater than 16!");
        return;
    }

    /* handle printing different bases */
    if (base == 2)
        print("0b");
    if (base == 16)
        print("0x");

    /* if the number is 0, just print 0. save some processing */
    if (number == 0)
    {
        print_char('0');
        return;
    }

    /*
     * otherwise convert the number into a string.
     * it'll be in reverse order so print it in reverse.
     */
    char buffer[300];
    int current_pos = 0;
    const char numtochar[] = "0123456789ABCDEF";
    do
    {
        buffer[current_pos] = numtochar[number % base];
        number /= base;
        ++current_pos;
    } while (number != 0 && current_pos < 300);

    int i;
    for (i = current_pos; i > 0; i--)
        print_char(buffer[i - 1]);
}

void printf(char *format, ...)
{
    va_list arg;

    va_start(arg, format);
    /*printf_impl(format, arg);*/
    while (*format != '\0')
    {

        /* Handle special characters */
        if (*format == '%')
        {
            if (*(format + 1) == '%')
            {
                print_char('%');
            }
            else if (*(format + 1) == 's')
            {
                char *subString = va_arg(arg, char *);
                print(subString);
            }
            else if (*(format + 1) == 'c')
            {
                char character_arg = va_arg(arg, int);
                print_char(character_arg);
            }
            else if (*(format + 1) == 'd')
            {
                unsigned long num = va_arg(arg, uint32_t);
                print_uint_to_string(num, 10);
            }
            else if (*(format + 1) == 'x')
            {
                unsigned long num = va_arg(arg, uint32_t);
                print_uint_to_string(num, 16);
            }
            else if (*(format + 1) == 'b')
            {
                unsigned int num = va_arg(arg, uint32_t);
                print_uint_to_string(num, 2);
            }
            else if (*(format + 1) == '\0')
            {
                print("printf error: next character is null");
                break;
            }
            else
            {
                print("printf error: Unknown escape sequence %");
                print_char(*(format + 1));
                break;
            }
            format++;
            /* Simply print the next character */
        }
        else
        {
            print_char(*format);
        }

        // Move to the next character
        format++;
    }
    va_end(arg);
}

// Compara 2 stringhe, ritorna 0 se sono uguali
int strcmp(const char *str1, const char *str2)
{
    int res = 0;
    while (!(res = *(unsigned char *)str1 - *(unsigned char *)str2) && *str2)
        ++str1, ++str2;

    if (res < 0)
        res = -1;
    if (res > 0)
        res = 1;

    return res;
}