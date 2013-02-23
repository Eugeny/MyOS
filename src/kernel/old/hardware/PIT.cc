#include <hardware/PIT.h>
#include <kutils.h>


void PIT::setFrequency(u32int freq) {
    u32int divisor = 1193180 / freq;

    outb(0x43, 0x36);

    u8int l = (u8int)(divisor & 0xFF);
    u8int h = (u8int)((divisor>>8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);
}

void PIT::setHandler(interrupt_handler h) {
    Interrupts::get()->setHandler(IRQ(0), h);
}
