#ifndef IO_H
#define IO_H

#include <stdint.h>

// Legge un byte da una porta
extern unsigned char insbyte(unsigned short port);

// Invia un byte ad una porta
extern void outbyte(unsigned short port, uint8_t byte);

// Mette in pausa la CPU per circa 3 microsecondi (su sistemi vecchi)
void io_wait();

// Abilita gli interrupts
extern void EnableInterrupts();

// Disabilita gli interrupts
extern void DisableInterrupts();

#endif