#include "kutil.h"
#include "alloc/malloc.h"

#include <vector>
#include <map>

#include <core/CPU.h>
#include <hardware/keyboard/Keyboard.h>
#include <hardware/cmos/CMOS.h>
#include <hardware/pit/PIT.h>
#include <interrupts/IDT.h>
#include <interrupts/Interrupts.h>
#include <memory/AddressSpace.h>
#include <memory/FrameAlloc.h>
#include <memory/Memory.h>
#include <tty/Terminal.h>
#include <tty/Escape.h>
#include <tty/PhysicalTerminalManager.h>


int main() {}


void pit_handler(isrq_registers_t* regs) {
    static int counter = 0;
    counter++;
    if (counter < 100)
        if (counter % 20 == 0)
            klog('t', "Timer %i!", counter);
    if (counter % 5 == 0)
        PhysicalTerminalManager::get()->render();
    microtrace();
}

void irq7_mute(isrq_registers_t* regs) {

}

void handlePF(isrq_registers_t* regs) {
    Memory::handlePageFault(regs);
}

void handleGPF(isrq_registers_t* regs) {
    klog('e', "GENERAL PROTECTION FAULT");
    klog('e', "Faulting code: %lx", regs->rip);
    klog_flush();
    for(;;);
}

void handleKbd(uint64_t mod, uint64_t scan) {
    //klog('i', "Keyboard %x %x", mod, scan);
    PhysicalTerminalManager::get()->dispatchKey(mod, scan);
}



extern "C" void kmain () {
    CPU::enableSSE();
    
    Memory::init();

    kalloc_switch_to_main_heap();

    klog_init();

    asm volatile("cli");
    asm volatile("clts");


    PhysicalTerminalManager::get()->init(5);
    klog_init_terminal();
    klog('i', "Kernel log started");
    PhysicalTerminalManager::get()->render();

    klog('i', "Setting IDT");
    IDT::get()->init();

    klog('i', "Time is %u", CMOS::get()->readTime());


    klog('i', "Configuring timer");
    PIT::get()->setFrequency(2500);
    Interrupts::get()->setHandler(IRQ(0), pit_handler);
    Interrupts::get()->setHandler(IRQ(7), irq7_mute);
    Interrupts::get()->setHandler(13, handleGPF);
    Interrupts::get()->setHandler(14, handlePF);

    Keyboard::get()->init();
    Keyboard::get()->setHandler(handleKbd);




    AddressSpace* as = AddressSpace::kernelSpace;

    as->allocateSpace(0x910000000, 0x10000, 0);
    *((uint64_t*)     0x910000000) = 5;

    as->dump();

    AddressSpace* ns = as->clone();
    ns->dump();
    for(;;);

    KTRACEMEM




    Terminal* t = PhysicalTerminalManager::get()->getActiveTerminal();

    t->write(Escape::C_B_RED);
    t->write(":K: ");
    t->write(Escape::C_B_GREEN);
    t->write(":G: ");
    t->write(Escape::C_B_BLUE);
    t->write(":B: ");
    t->write(Escape::C_B_YELLOW);
    t->write(":Y: ");
    t->write(Escape::C_OFF);
    t->write("test\n");
    t->render();

    for(;;) {
        //for (int i = 0; i < 10000000; i++);
          //  t->write(".");
    }
}