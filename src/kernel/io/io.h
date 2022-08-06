#ifndef IO_H
#define IO_H

#include <stdint.h>

// Legge un byte da una porta
extern uint8_t insbyte(unsigned short port);
extern uint16_t insword(unsigned short port);

// Invia un byte ad una porta
extern void outbyte(unsigned short port, uint8_t byte);
extern void outword(unsigned short port, uint16_t byte);

// Mette in pausa la CPU per circa 3 microsecondi (su sistemi vecchi)
void io_wait();

// Abilita gli interrupts
extern void EnableInterrupts();

// Disabilita gli interrupts
extern void DisableInterrupts();

#endif