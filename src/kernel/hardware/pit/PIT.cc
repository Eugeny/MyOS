#include <hardware/pit/PIT.h>
#include <hardware/io.h>


void PIT::setFrequency(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);
}

void PIT::setHandler(interrupt_handler h) {
    Interrupts::get()->setHandler(IRQ(0), h);
}
