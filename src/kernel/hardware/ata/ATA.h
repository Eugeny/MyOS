#ifndef HARDWARE_ATA_ATA_H
#define HARDWARE_ATA_ATA_H

#include <lang/lang.h>

void ata_init();
void ata_read(uint64_t lba, uint8_t* buf);
void ata_write(uint64_t lba, uint8_t* buf);

#endif