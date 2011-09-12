#include <hardware/PIT.h>
#include <kutils.h>


void PIT::setFrequency(u32int freq) {
    klog("t");
    u32int divisor = 1193180 / freq;
    klog("t");

    outb(0x43, 0x36);
    klog("t");

    u8int l = (u8int)(divisor & 0xFF);
    klog("t");
    u8int h = (u8int)((divisor>>8) & 0xFF);
    klog("t");

    outb(0x40, l);
    klog("t");
    outb(0x40, h);
    klog("e");
}

void PIT::setHandler(interrupt_handler h) {
    Interrupts::get()->setHandler(IRQ(0), h);
}
