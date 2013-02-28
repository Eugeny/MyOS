#include <hardware/pit/PIT.h>
#include <core/MQ.h>
#include <hardware/io.h>
#include <kutil.h>


const char* PIT::MSG_TIMER = "timer";


static void handler(isrq_registers_t* regs) {
    MQ::post(PIT::MSG_TIMER, (void*)regs);
}

void PIT::init() {
    MQ::registerMessage(MSG_TIMER);
    Interrupts::get()->setHandler(IRQ(0), handler);
}

void PIT::setFrequency(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);
}
