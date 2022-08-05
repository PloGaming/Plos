#ifndef STRING_H
#define STRING_H

#include <strings/string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <terminal/terminal.h>

int strlen(char *string);
void printf(char *format, ...);
int strcmp(const char *str1, const char *str2);

#endif