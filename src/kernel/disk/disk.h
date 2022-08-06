#ifndef DISK_H
#define DISK_H

#define ATA_MASTER 0xE0
#define ATA_READ_SECTORS 0x20
#define ATA_CAN_READ 1

#define SECTOR_SIZE 512

typedef uint32_t PLOS_DISK_TYPE;

// Rappresenta un disco fisico
#define PLOS_DISK_TYPE_REAL 0

struct disk
{
	PLOS_DISK_TYPE type;
	int sector_size;
};

int disk_read_sector(unsigned int lba, int total, void *buf);
void read_is_ready();
void disk_search_and_init();

#endif