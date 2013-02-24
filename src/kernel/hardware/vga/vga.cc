#include <lang/lang.h>
#include <unistd.h>
#include <hardware/io.h>
#include <hardware/vga/vga.h>

#define WIDTH 80

void vga_move_cursor(int x, int y) {
    uint16_t position = (y * WIDTH) + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}
