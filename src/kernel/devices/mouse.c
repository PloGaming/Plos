#include <devices/mouse.h>
#include <strings/string.h>

static struct mouse ps2mouse;

// Handler dell'interrupt per il mouse (TODO)
void mouse_handler()
{
	return;
}

// Inizializza il mouse
void mouse_initialize()
{
	// Impostiamo dei valori iniziali per il nostro mouse
	ps2mouse.offset = 0;
	ps2mouse.current_state = 0;

	// Abilita gli interrupt PS2 (dovrebbero essere gia attivi,
	// ma fa lo stesso... w la sicurezza )
	outbyte((uint16_t)PS2_WRITEREGISTER, (uint8_t)0xA8);

	// Scrive il "Controller Configuration Byte" e ci
	// rende possibile la lettura di esso dalla porta 0x60
	outbyte((uint16_t)PS2_WRITEREGISTER, (uint8_t)0x20);

	// Leggiamo il byte inserito prima
	uint8_t ControllerConfByte = insbyte((uint16_t)PS2_DATAPORT);

	// Modifichiamolo
	ControllerConfByte |= 2;

	// Allertiamo il PS2 che stiamo per scrivere il byte nella data port
	outbyte((uint16_t)PS2_WRITEREGISTER, (uint8_t)0x60);

	// Scriviamolo
	outbyte((uint16_t)PS2_DATAPORT, ControllerConfByte);

	outbyte((uint16_t)PS2_WRITEREGISTER, 0xD4);
	outbyte((uint16_t)PS2_DATAPORT, 0xF4);

	outbyte((uint16_t)PS2_DATAPORT, (uint8_t)0xF4);
	if (insbyte((uint16_t)PS2_DATAPORT) != MOUSE_ACKNOWLEDGE)
	{
		kernelPanic("Il mouse non funziona\n");
	}
}
