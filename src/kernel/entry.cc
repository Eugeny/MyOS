#include "kutil.h"
#include "alloc/malloc.h"

#include <core/CPU.h>
#include <core/Debug.h>
#include <core/MQ.h>
#include <core/Process.h>
#include <core/Scheduler.h>
#include <core/Wait.h>
#include <hardware/keyboard/Keyboard.h>
#include <hardware/cmos/CMOS.h>
#include <hardware/io.h>
#include <hardware/pit/PIT.h>
#include <hardware/vga/VGA.h>
#include <interrupts/IDT.h>
#include <interrupts/Interrupts.h>
#include <memory/AddressSpace.h>
#include <memory/Memory.h>
#include <syscall/Syscalls.h>
#include <tty/PhysicalTerminalManager.h>

#include <fs/devfs/DevFS.h>
#include <fs/devfs/PTY.h>
#include <fs/fat32/FAT32FS.h>
#include <fs/procfs/ProcFS.h>
#include <fs/vfs/VFS.h>
#include <fs/File.h>
#include <fs/Directory.h>

#include <elf/ELF.h>
#include <multiboot.h>

#include <hardware/pm.h>


int main() { return 0; }


void isrq6(isrq_registers_t* regs)  {
    klog('e', "Invalid opcode, rip=%lx", regs->rip);
    Debug::MSG_DUMP_REGISTERS.post(NULL);
    Scheduler::get()->logTask();
    AddressSpace::current->dump();
    klog_flush();
    for(;;);
}

void isrq0(isrq_registers_t* regs)  {
    klog('e', "Division by zero, rip=%lx", regs->rip);
    Debug::MSG_DUMP_REGISTERS.post(NULL);
    Scheduler::get()->logTask();
    klog_flush();
    for(;;);
}

void repainterThread(void*) {
    for (;;) {
        Scheduler::get()->getActiveThread()->wait(new WaitForDelay(50));
        PhysicalTerminalManager::get()->render();
        //microtrace();
    }
}




extern "C" void kmain (multiboot_info_t* mbi) {
    __output("Starting up...", 0);

    CPU::enableSSE();
    CPU::CLI();
    CPU::CLTS();
    
    __output("Initializing paging...", 80);
    Memory::init();
    
    __output("Initializing heap...", 160);
    kalloc_switch_to_main_heap();
    klog_init();

    __output("Initializing VGA...", 240);
    VGA::enableHighResolution();

    PhysicalTerminalManager::get()->init(5);
    klog_init_terminal();
    klog('s', "Kernel log started");

    PhysicalTerminalManager::get()->render();

    Debug::init();
    IDT::get()->init();

    PIT::get()->init();
    PIT::get()->setFrequency(25);

    Keyboard::get()->init();
    Interrupts::get()->setHandler(IRQ(7),  INTERRUPT_MUTE);
    Interrupts::get()->setHandler(IRQ(14), INTERRUPT_MUTE);
    Interrupts::get()->setHandler(IRQ(15), INTERRUPT_MUTE);
    Interrupts::get()->setHandler(0x00, isrq0);
    Interrupts::get()->setHandler(0x06, isrq6);
   

    Syscalls::init();
    

    klog('i', "Starting scheduler");
    Scheduler::get()->init();
    klog_flush();
    Scheduler::get()->spawnKernelThread(&repainterThread, "repainter");
    Scheduler::get()->resume();


    // -------------------------------------------------

    klog('i', "");
    klog('i', "Setting up filesystem:");
    auto vfs = VFS::get();
    vfs->mount("/", new FAT32FS());
    vfs->mount("/dev", new DevFS());
    vfs->mount("/proc", new ProcFS());
    klog('s', "Filesystem ready");


    klog('i', "");
    klog('i', "Loading init");
    Process* p = Scheduler::get()->spawnProcess(Scheduler::get()->kernelProcess, "init");

    PTY* pty = PhysicalTerminalManager::get()->getPTY(0);

    p->pty = pty;
    p->attachFile(pty->openSlave());
    p->attachFile(pty->openSlave());
    p->attachFile(pty->openSlave());

    auto elf = new ELF();
    elf->loadFromFile("/bin/init");
    elf->loadIntoProcess(p);

    char** argv = new char*[2];
    argv[0] = "/bin/init";
    argv[1] = NULL;

    klog('i', "Starting init");
    elf->startMainThread(p, argv, NULL);
    Debug::tracingOn = true;
    Scheduler::get()->resume();

    klog('s', "Init is running");
    CPU::STI();
    for (;;)
        CPU::halt();
}