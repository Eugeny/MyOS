#include <hardware/cmos/CMOS.h>
#include <hardware/io.h>


uint8_t CMOS::readRegister(uint8_t index) {
    outb(0x70, (1 << 7) | (index));
    uint8_t d = inb(0x71);
    return (d & 0xf) + d / 16 * 10;
}

time_t CMOS::readTime() {
    time_t time = 0;
    time += readRegister(0x00);
    time += readRegister(0x02) * 60;
    time += readRegister(0x04) * 3600;
    time += readRegister(0x07) * 3600 * 24;
    time += readRegister(0x08) * 3600 * 24 * 31;
    time += (30 + readRegister(0x09)) * 3600 * 24 * 365;
    return time;
}