#include <hardware/cmos/CMOS.h>
#include <hardware/io.h>


uint8_t CMOS::readRegister(uint8_t index) {
    outb(0x70, (1 << 7) | (index));
    return inb(0x71);
}

uint64_t CMOS::readTime() {
    uint64_t time = 0;
    time += readRegister(0x00);
    time += readRegister(0x02) * 60;
    time += readRegister(0x04) * 3600;
    time += readRegister(0x07) * 3600 * 24;
    time += readRegister(0x08) * 3600 * 24 * 31;
    time += readRegister(0x09) * 3600 * 24 * 31 * 12;
    return time;
}