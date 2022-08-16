#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>

// Struttura che definisce le porte dei device ATA
struct ATA_Device
{
	uint16_t dataPort;
	uint8_t errorPort;
	uint8_t sectorCountPort;
	uint8_t lbaLowPort;
	uint8_t lbaMidPort;
	uint8_t lbaHighPort;
	uint8_t devicePort;
	uint8_t commandPort;
	uint8_t controlPort;

	bool isMaster;
};

void Initialize_ATA_Device(struct ATA_Device *ata, uint16_t basePort, bool isMaster);
void Identify(struct ATA_Device *ata);
void Read28(struct ATA_Device *ata, uint32_t sector, uint8_t *buffer, int count);
void Write28(struct ATA_Device *ata, uint32_t sector, uint8_t *buffer, int count);
void Flush();

#endif