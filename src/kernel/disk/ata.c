#include <disk/ata.h>
#include <io/io.h>
#include <strings/string.h>

// Inizializza un dispositivo ata con le corrispettive porte
void Initialize_ATA_Device(struct ATA_Device *ata, uint16_t basePort, bool isMaster)
{
	// Bool che distingue slave da master
	ata->isMaster = isMaster;

	ata->dataPort = basePort;
	ata->errorPort = basePort + 1;
	ata->sectorCountPort = basePort + 2;
	ata->lbaLowPort = basePort + 3;
	ata->lbaMidPort = basePort + 4;
	ata->lbaHighPort = basePort + 5;
	ata->devicePort = basePort + 6;
	ata->commandPort = basePort + 7;
	ata->controlPort = basePort + 0x206;
}

// Mostra sullo schermo delle informazioni sul dispositivo ATA
void Identify(struct ATA_Device *ata)
{
	outbyte(ata->devicePort, ata->isMaster ? 0xA0 : 0xB0);
	outbyte(ata->controlPort, 0);

	outbyte(ata->devicePort, 0xA0);
	uint8_t status = insbyte(ata->commandPort);
	if (status == 0xFF)
	{
		printf("No device!\n");
		return;
	}

	outbyte(ata->devicePort, ata->isMaster ? 0xA0 : 0xB0);
	outbyte(ata->sectorCountPort, 0);
	outbyte(ata->lbaLowPort, 0);
	outbyte(ata->lbaMidPort, 0);
	outbyte(ata->lbaHighPort, 0);
	outbyte(ata->commandPort, 0xEC);

	status = insbyte(ata->commandPort);
	if (status == 0x00)
	{
		printf("No device!\n");
		return;
	}

	while (((status & 0x80) == 0x80) && (status & 0x01) != 0x01)
		status = insbyte(ata->commandPort);

	if (status & 0x01)
	{
		printf("ERROR in ata.c\n");
		return;
	}

	for (uint16_t i = 0; i < 256; i++)
	{
		uint16_t data = insbyte(ata->dataPort);
		char *buf = "  \0";
		buf[1] = (data >> 8) & 0x00FF;
		buf[0] = (data & 0x00FF);
		printf(buf);
	}
}

void Read28(struct ATA_Device *ata, uint32_t sector, uint8_t *buffer, int count)
{
}

void Write28(struct ATA_Device *ata, uint32_t sector, uint8_t *buffer, int count)
{
}

void Flush()
{
}