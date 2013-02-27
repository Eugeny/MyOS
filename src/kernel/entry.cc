#include "kutil.h"
#include "alloc/malloc.h"

#include <vector>
#include <map>

#include <core/CPU.h>
#include <core/Process.h>
#include <core/Thread.h>
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



void irq7_mute(isrq_registers_t* regs) {

}

void handlePF(isrq_registers_t* regs) {
    Memory::handlePageFault(regs);
}

void handleGPF(isrq_registers_t* regs) {
    klog('e', "GENERAL PROTECTION FAULT");
    klog('e', "Faulting code: %lx", regs->rip);
    klog('e', "Errcode      : %lx", regs->err_code);
    klog_flush();
}

void handleKbd(uint64_t mod, uint64_t scan) {
    //klog('i', "Keyboard %x %x", mod, scan);
    PhysicalTerminalManager::get()->dispatchKey(mod, scan);
}



bool taskingActive = false;
Thread* threads[3];
int activeThread = 0;

void pit_handler(isrq_registers_t* regs) {
    static int counter = 0;
    counter++;
    if (counter % 5 == 0)
        PhysicalTerminalManager::get()->render();

    microtrace();

    if (taskingActive) {
        threads[activeThread]->storeState(regs);
        activeThread = counter % 2 + 1;
        threads[activeThread]->recoverState(regs);
    }
}

void handleDebug(isrq_registers_t* regs) {
    klog('w', "Register dump");
    klog('w', "RIP: %016lx   RSP: %016lx", regs->rip, regs->rsp);
    klog('w', "RDI: %016lx   RSI: %016lx", regs->rdi, regs->rsi);
    klog('w', "RAX: %016lx   RBX: %016lx", regs->rax, regs->rbx);
    klog('w', "RCX: %016lx   RDX: %016lx", regs->rcx, regs->rdx);
    klog_flush();
}

void handleSaveKernelState(isrq_registers_t* regs) {
    klog('d', "Saving kernel thread state");
    threads[0]->storeState(regs);
}

void threadA() {
    for (;;) {
        for (int i = 0; i < 1000000; i++);
        klog('i', "Thread A");
    }
}

void threadB() {
    volatile int b = 2;
    for (;;) {
        for (int i = 0; i < 1000000; i++);
        klog('w', "Thread B");
    }
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
    Interrupts::get()->setHandler(IRQ(7), irq7_mute);
    Interrupts::get()->setHandler(13, handleGPF);
    Interrupts::get()->setHandler(14, handlePF);
    Interrupts::get()->setHandler(0x7f, handleSaveKernelState);
    Interrupts::get()->setHandler(0xff, handleDebug);
    Interrupts::get()->setHandler(IRQ(0), pit_handler);

    Keyboard::get()->init();
    Keyboard::get()->setHandler(handleKbd);


    Process* p = new Process();
    p->addressSpace = AddressSpace::kernelSpace;
    p->addressSpace->dump();

    threads[0] = new Thread(p);

    asm volatile("int $0x7f");
//    asm volatile("int $0xff");

    threads[1] = new Thread(p);
    threads[2] = new Thread(p);
    threads[1]->state.regs = threads[0]->state.regs;
    threads[2]->state.regs = threads[0]->state.regs;
    threads[1]->createStack(0x2000);
    threads[2]->createStack(0x2000);
    threads[1]->state.regs.rip = (uint64_t)&threadA;
    threads[2]->state.regs.rip = (uint64_t)&threadB;

    taskingActive = true;

    for(;;);



}