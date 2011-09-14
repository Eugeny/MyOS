#include <hardware/Disk.h>
#include <hardware/ATA.h>
#include <interrupts/Interrupts.h>
#include <kutils.h>


static void dummy(isrq_registers_t r) {}

void Disk::init() {
    u8int buf[512];

    Interrupts::get()->setHandler(IRQ(14), dummy);

    ata_read(0, buf);
    base =
        buf[0x1be + 8] +
        buf[0x1be + 9] * 256 +
        buf[0x1be + 10] * 65536 +
        buf[0x1be + 11] * 65536 * 256;

    DEBUG(to_hex(base));
}

void Disk::read(u32int lba, void* buf) {
    ata_read(base+lba, (u8int*)buf);
}
