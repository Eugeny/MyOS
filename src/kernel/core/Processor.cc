#include <core/Processor.h>


u32int Processor::getStackPointer() {
    u32int esp;
    asm volatile ("mov %%esp, %0" : "=r" (esp));
    return esp + 0x1c;
}

void Processor::disableInterrupts() {
    asm volatile("cli");
}

void Processor::enableInterrupts() {
    asm volatile("sti");
}
