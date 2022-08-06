#include <devices/pit.h>
#include <io/io.h>
#include <limits.h>
#include <strings/string.h>

// Contatore globale di tick
static uint32_t global_tick_count = 0;

// Interrupt handler per IRQ[0]
// incrementa solamente di 1 il global_tick_count
void pit_interrupt_handler()
{
	// Voglio evitare un overflow
	if (global_tick_count == UINT32_MAX)
	{
		global_tick_count = 0;
		printf("Global tick count resettato\n");
	}

	// Aumenta il numero di tick
	global_tick_count++;
}

// Funzione che invia un comando al PIT
void pit_send_cmd(uint8_t cmd)
{
	outbyte(I86_PIT_REG_COMMAND, cmd);
}

// Funzione che invia dati al PIT
void pit_send_data(uint16_t data, uint8_t counter)
{
	uint8_t port = (counter == I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 : ((counter == I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);
	outbyte(port, data);
}

// Funzione che legge dati dal PIT
uint8_t pit_read_data(uint16_t counter)
{
	uint8_t port = (counter == I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 : ((counter == I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);
	return insbyte(port);
}

// Inizia il count
void pit_start_counter(uint32_t freq, uint8_t counter, uint8_t mode)
{
	if (freq == 0)
		return;

	uint16_t divisor = 1193180 / freq;

	uint8_t ocw = 0;
	ocw = (ocw & ~I86_PIT_OCW_MASK_MODE) | mode;
	ocw = (ocw & ~I86_PIT_OCW_MASK_RL) | I86_PIT_OCW_RL_DATA;
	ocw = (ocw & ~I86_PIT_OCW_MASK_COUNTER) | counter;
	pit_send_cmd(ocw);

	pit_send_data(divisor & 0xff, 0);
	pit_send_data((divisor >> 8) & 0xff, 0);

	// Resetta il numero di tick
	global_tick_count = 0;
}

// Ritorna il numero corrente di tick
uint32_t get_tick_count()
{
	return global_tick_count;
}