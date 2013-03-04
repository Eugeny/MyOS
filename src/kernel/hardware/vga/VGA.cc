#include <lang/lang.h>
#include <unistd.h>
#include <hardware/io.h>
#include <hardware/vga/VGA.h>


extern "C" void set_text_mode(int hi_res);

#include "_vga_internal.cc"

int VGA::width = 0;
int VGA::height = 0;

void VGA::moveCursor(int x, int y) {
    uint16_t position = (y * width) + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

void VGA::enableHighResolution() {
    _vga_internal_set_text_mode();
    width = 90;
    height = 60;
}