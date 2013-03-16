#include <hardware/ata/ATA.h>
#include <alloc/malloc.h>
#include <hardware/io.h>
#include <core/CPU.h>
#include <string.h>
#include <kutil.h>


#define CACHE_SIZE 10
#define CACHE_BLOCK_SIZE 512

static uint64_t ata_cache_offsets[CACHE_SIZE];
static uint64_t ata_cache_timing[CACHE_SIZE];
static uint8_t* ata_cache_content[CACHE_SIZE * CACHE_BLOCK_SIZE];

#define CACHEPTR(id) ((uint8_t*)((uint64_t)ata_cache_content + id * CACHE_BLOCK_SIZE))


void ata_init() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        ata_cache_offsets[i] = -1;
        ata_cache_timing[i] = 999;
    }
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
    for (int i = 0; i < CACHE_SIZE; i++)
        if (ata_cache_offsets[i] == lba && ata_cache_offsets[i] != -1) {
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
        //klog('t', "ATA cache hit %lx", lba);
        //return;
    //}
    klog('t', "ATA cache miss %lx", lba);

    int c, h, s;
    lba2chs(lba, &c, &h, &s);

    CPU::CLI();

    outb(0x1f6, 0xa0 + h);
    outb(0x1f2, 1);
    outb(0x1f3, (char)s);
    outb(0x1f4, (char)(c%256));
    outb(0x1f5, (char)(c/256));
    outb(0x1f7, 0x20);

    bool ready = false;
    do {
        uint8_t st = inb(0x1f7);
        ready = st & 8;
    } while (!ready);

    asm("rep insw" : : "c"(256), "d"(0x1f0), "D"(buf));

    ata_cache_set(lba, buf);

    CPU::STI();
}


void ata_write(uint64_t lba, uint8_t* buf) {
    int c, h, s;
    lba2chs(lba, &c, &h, &s);

    klog('t', "ATA write %lx", lba);
    ata_cache_invalidate(lba);

    outb(0x1f6, 0xa0 + h);
    outb(0x1f2, 1);
    outb(0x1f3, (char)s);
    outb(0x1f4, (char)(c%256));
    outb(0x1f5, (char)(c/256));
    outb(0x1f7, 0x30);

    bool ready = false;
    do {
        uint8_t st = inb(0x1f7);
        ready = st & 8;
    } while (!ready);

    asm("rep outsw" : : "c"(256), "d"(0x1f0), "S"(buf));
}
