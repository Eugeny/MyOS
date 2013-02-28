#include <hardware/pit/PIT.h>
#include <hardware/io.h>
#include <kutil.h>


Message PIT::MSG_TIMER("timer");


static uint64_t ticks = 0;

static void handler(isrq_registers_t* regs) {
    ticks++;
    PIT::MSG_TIMER.post((void*)regs);
}

void PIT::init() {
    Interrupts::get()->setHandler(IRQ(0), handler);
}

uint64_t PIT::getTicks() {
    return ticks;
}

uint64_t PIT::getTime() {
    return ticks * 1000 / frequency;
}

uint32_t PIT::getFrequency() {
    return frequency;
}

void PIT::setFrequency(uint32_t freq) {
    frequency = freq;
    uint32_t divisor = 1193180 / freq;

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);
}
