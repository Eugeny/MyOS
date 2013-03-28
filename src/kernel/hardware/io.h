#ifndef HARDWARE_IO_H
#define HARDWARE_IO_H

#include <lang/lang.h>

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);

#endif