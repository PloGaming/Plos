#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <kernel.h>

#define MOUSE_ACKNOWLEDGE 0xFA

// Definisce lo stato del mouse
struct mouse
{
	// Buffer che conterra i dati inviati dal mouse (1 pacchetto sono 3 byte)
	uint8_t buffer[3];

	// Offset relativo al buffer per salvare i dati del pacchetto
	uint8_t offset;

	// Conterra delle informazioni sullo stato del mouse, in questa maniera
	// potremo confrontarlo con pacchetti successivi per vedere  un cambiamento
	uint8_t current_state;
};

void mouse_handler();
void mouse_initialize();

#endif