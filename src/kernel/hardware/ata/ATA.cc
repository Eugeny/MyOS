#include <hardware/ata/ATA.h>
#include <alloc/malloc.h>
#include <hardware/io.h>
#include <core/CPU.h>
#include <string.h>
#include <kutil.h>

#define CACHE_SIZE 129

#define CACHE_BLOCK_SIZE 512

uint64_t ata_cache_offsets[CACHE_SIZE];
uint64_t ata_cache_timing[CACHE_SIZE];
uint8_t ata_cache_content[CACHE_SIZE * CACHE_BLOCK_SIZE];
//uint8_t* ata_cache_content;

#define CACHEPTR(id) ((uint8_t*)((uint64_t)ata_cache_content + id * CACHE_BLOCK_SIZE))


void ata_init() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        ata_cache_offsets[i] = -1;
        ata_cache_timing[i] = 999;
    }
    //ata_cache_content = (uint8_t*)kvalloc(CACHE_BLOCK_SIZE * CACHE_SIZE);
}


static void ata_cache_set(uint64_t lba, uint8_t* content) {
    int best = 0;
    for (int i = 1; i < CACHE_SIZE; i++)
        if (ata_cache_timing[i] > ata_cache_timing[best])
            best = i;

    ata_cache_timing[best] = 0;
    ata_cache_offsets[best] = lba;
    memcpy(CACHEPTR(best), content, CACHE_BLOCK_SIZE);

    for (int i = 0; i < CACHE_SIZE; i++)
        ata_cache_timing[i]++;
}

static bool ata_cache_get(uint64_t lba, uint8_t* buffer) {
    klog('t', "ATA cache get %lx into %lx", lba, buffer);
    for (int i = 0; i < CACHE_SIZE; i++)
        if (ata_cache_offsets[i] == lba) {
            klog('t', "ATA cache line %i hit lba %i into buffer %lx", i, lba, buffer);
            return false;

            memcpy(buffer, CACHEPTR(i), CACHE_BLOCK_SIZE);
            ata_cache_timing[i] = 0;
            return true;
        }
    return false;
}

static void ata_cache_invalidate(uint64_t lba) {
    for (int i = 0; i < CACHE_SIZE; i++)
        if (ata_cache_offsets[i] <= lba && ata_cache_offsets[i] > lba - CACHE_BLOCK_SIZE && ata_cache_offsets[i] != -1) {
            ata_cache_offsets[i] = 0;
        }
}

void lba2chs (uint64_t lba, int *c, int *h, int *s) {
    *c = lba/(63*16);
    *h = (lba/63) % 16;
    *s = (lba % 63) + 1;
}


void ata_read(uint64_t lba, uint8_t* buf) {
    //if (ata_cache_get(lba, buf)) {
    //    klog('t', "ATA cache hit %lx", lba);
    //    return;
    //}
    klog('t', "ATA cache miss %lx", lba);
    //klog_flush();

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

    //ata_cache_set(lba, buf);
}


void ata_write(uint64_t lba, uint8_t* buf) {
    int c, h, s;
    lba2chs(lba, &c, &h, &s);
 
    //ata_cache_invalidate(lba);

    outb(0x1f6, 0xa0 + h);
    outb(0x1f2, 1);
    outb(0x1f3, (char)s);
    outb(0x1f4, (char)(c%256));
    outb(0x1f5, (char)(c/256));
    outb(0x1f7, 0x30);

    while ((inb(0x1f7) & 8) == 0);

    asm("rep outsw" : : "c"(256), "d"(0x1f0), "S"(buf));
    //klog('w', "ATA write %lx", lba);
    // wait ):
    klog_flush();
}
