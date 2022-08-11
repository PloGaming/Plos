#include <io/io.h>
#include <disk/disk.h>
#include <strings/string.h>
#include <memory/memory.h>

struct disk disk;

// Variabile che indica se ATA puo leggere
static uint8_t can_read = '\0';

// Funzione che legge total settori, partendo dall'LBA
// (Logical Block Sector) e salva i contenuti in buf
int disk_read_sector(unsigned int lba, int total, void *buf)
{
	// 28 bit lba

	// 1_ Dobbiamo ORare ATA_MASTER per selezionare il master drive
	// con i primi 4 bit dell'lba
	outbyte(0x1F6, (lba >> 24) | ATA_MASTER);

	// 2_ Inviare byte NULL alla porta 0x1F1
	// Sconsigliato, dato che è inutile e facoltativo
	// outbyte(0x1F1, NULL);

	// 3_ Inviare il numero di blocchi da leggere
	// alla porta 0x1F2
	outbyte(0x1F2, (uint8_t)total);

	// 4_ Invia gli ultimi 8 bit dell'lba
	outbyte(0x1F3, (uint8_t)(lba & 0xff));

	// 5_ Invia gli 8 bit prima dell'lba
	outbyte(0x1F4, (uint8_t)(lba >> 8));

	// 6_ Invia gli 8 bit prima prima
	outbyte(0x1F5, (uint8_t)(lba >> 16));

	// 7_ Invia il comando per leggere i settori
	outbyte(0x1F7, ATA_READ_SECTORS);

	unsigned short *data = (unsigned short *)buf;

	for (int i = 0; i < total; i++)
	{
		// 8_ Aspetta che arrivi un IRQ
		while (!can_read)
			;

		can_read = '\0';

		// 9_ Copia dall'hard disk alla memoria 256 valori 16-bit
		for (int i = 0; i < 256; i++)
		{
			*data = insword(0x1F0);
			data++;
		}
	}

	return 0;
}

// Funzione invocata da IRQ per dire che puo leggere
void read_is_ready()
{
	can_read = ATA_CAN_READ;
}

// Inizializza i vari dischi (per il momento no)
void disk_search_and_init()
{
	memset(&disk, '\0', sizeof(struct disk));

	disk.type = PLOS_DISK_TYPE_REAL;
	disk.sector_size = SECTOR_SIZE;
}

// Restituisce un disco dato un index (per il momento no)
struct disk *disk_get(int index)
{
	if (index != PLOS_DISK_TYPE_REAL)
		return NULL;

	return &disk;
}

// Legge "total" blocchi da un particolare disco
int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf)
{
	if (idisk != &disk)
		return -1;

	return disk_read_sector(lba, total, buf);
}