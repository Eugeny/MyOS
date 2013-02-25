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
    if (counter > 100)return;
    if (counter % 20 == 0)
        klog('t', "Timer %i!", counter);
    if (counter % 25 == 0)
        PhysicalTerminalManager::get()->render();
    microtrace();
}

extern "C" void kmain () {
    memory_initialize_default_paging();
 
    char* ptr = 0;
    for (char c = 0; c < 20; c++) {
        volatile char data = *(ptr + c * 1024*1024);
        *((char *)0xb8000) = ('0'+c);
    }

    volatile char data = *((uint64_t*)0xffffffffffffffe0);
for(;;);

    asm volatile("cli");


    //klog('i', "Setting IDT");
    IDT idt;
    //idt.init();
    sout("idt ok");

    
    float a = 0.1;
    PhysicalTerminalManager::get()->init(5);

    uint64_t cr0;
    asm volatile(" mov %%cr0, %0": "=r"(cr0));
    cr0 |= 1 << 2;
    cr0 |= 1 << 9;
    cr0 |= 1 << 10;
    asm volatile(" mov %0, %%cr0":: "r"(cr0));

    for (char c = 0; c < 32; c++) {
        *((char *)0xb8000 + c * 2) = (cr0 % 2 == 1)? '1': '0';
        *((char *)0xb8000 + c * 2 + 1) = 0x0f;
        cr0 /=2;
    }

    //for (;;);


    //microtrace();
    klog('t',"test");

    PhysicalTerminalManager::get()->init(5);
    klog_init();
    klog('i', "Kernel log started");
    klog('i', "Time is %u", CMOS::get()->readTime());


    klog('i', "Configuring timer");
    PIT::get()->setFrequency(2500);
    Interrupts::get()->setHandler(IRQ(0), pit_handler);

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