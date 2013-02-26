#include "kutil.h"
#include "alloc/malloc.h"

#include <vector>
#include <map>

#include <hardware/cmos/CMOS.h>
#include <hardware/pit/PIT.h>
#include <interrupts/IDT.h>
#include <interrupts/Interrupts.h>
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
    if (counter % 25 == 0)
        PhysicalTerminalManager::get()->render();
    microtrace();
}

void irq7_mute(isrq_registers_t* regs) {

}

extern "C" void kmain () {
    memory_initialize_default_paging();
 
    klog_init();

    volatile char data = *((uint64_t*)0xffffffffffffffe0);

    asm volatile("cli");
    asm volatile("clts");


    // ENABLE SSE/MMX
    uint64_t cr4;
    asm volatile(" mov %%cr4, %0": "=r"(cr4));
    cr4 |= 1 << 8;
    cr4 |= 1 << 9;
    asm volatile(" mov %0, %%cr4":: "r"(cr4));
    // --------------


    PhysicalTerminalManager::get()->init(5);
    klog_init_terminal();
    klog('i', "Kernel log started");

    klog('i', "Setting IDT");
    IDT idt;
    idt.init();

    //microtrace();
    klog('t',"test");

    PhysicalTerminalManager::get()->render();
    klog('i', "Time is %u", CMOS::get()->readTime());


    klog('i', "Configuring timer");
    PIT::get()->setFrequency(2500);
    Interrupts::get()->setHandler(IRQ(0), pit_handler);
    Interrupts::get()->setHandler(IRQ(7), irq7_mute);

    KTRACEMEM
    //auto i = new std::map<std::string,std::string> { { "a", "b" }, { "c", "d"} };
    //auto i = new std::map<char*,char*> { { "a", "b" }, { "c", "d"} };
    //KTRACEMEM
    //delete i;
    //KTRACEMEM

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
        for (int i = 0; i < 10000000; i++);
            t->write(".");
    }
}