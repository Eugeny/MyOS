#include <hardware/ata/ATA.h>
#include <alloc/malloc.h>
#include <hardware/io.h>
#include <core/CPU.h>
#include <string.h>
#include <kutil.h>

void lba2chs (uint64_t lba, int *c, int *h, int *s) {
    *c = lba/(63*16);
    *h = (lba/63) % 16;
    *s = (lba % 63) + 1;
}


void ata_init() {

}

void ata_read(uint64_t lba, uint8_t* buf) {
//    klog('t', "ATA cache miss %lx", lba);

    int c, h, s;
    lba2chs(lba, &c, &h, &s);

    outb(0x1f6, 0xa0 + h);
    outb(0x1f2, 1);
    outb(0x1f3, (char)s);
    outb(0x1f4, (char)(c%256));
    outb(0x1f5, (char)(c/256));
    outb(0x1f7, 0x20);

    while ((inb(0x1f7) & 8) == 0);

    asm("rep insw" : : "c"(256), "d"(0x1f0), "D"(buf));
}


void ata_write(uint64_t lba, uint8_t* buf) {
    int c, h, s;
    lba2chs(lba, &c, &h, &s);
 
    outb(0x1f6, 0xa0 + h);
    outb(0x1f2, 1);
    outb(0x1f3, (char)s);
    outb(0x1f4, (char)(c%256));
    outb(0x1f5, (char)(c/256));
    outb(0x1f7, 0x30);

    while ((inb(0x1f7) & 8) == 0);

    asm("rep outsw" : : "c"(256), "d"(0x1f0), "S"(buf));
    klog_flush();
}
