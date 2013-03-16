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



int main() { return 0; }

void isrq80(isrq_registers_t* regs)  {
    klog('w', "SYSCALL #%x", regs->rax);
    klog_flush();
}

void isrq6(isrq_registers_t* regs)  {
    klog('e', "Invalid opcode, rip=%lx", regs->rip);
    Debug::MSG_DUMP_REGISTERS.post(NULL);
    klog_flush();
    for(;;);
}

void isrq0(isrq_registers_t* regs)  {
    klog('e', "Division by zero, rip=%lx", regs->rip);
    Debug::MSG_DUMP_REGISTERS.post(NULL);
    klog_flush();
    for(;;);
}

void repainterThread(void*) {
    for (;;) {
        Scheduler::get()->getActiveThread()->wait(new WaitForDelay(50));
        PhysicalTerminalManager::get()->render();
        microtrace();
    }
}

void testThread(void*) {
    for (;;) {
        Scheduler::get()->getActiveThread()->wait(new WaitForDelay(3000));
        KTRACEMEM
    }
}




extern "C" void kmain (multiboot_info_t* mbi) {
    CPU::enableSSE();
    CPU::CLI();
    CPU::CLTS();
    
    Memory::init();
    kalloc_switch_to_main_heap();
    klog_init();


    VGA::enableHighResolution();

    PhysicalTerminalManager::get()->init(5);
    klog_init_terminal();
    klog('w', "");
    klog('w', "Kernel log started");

    PhysicalTerminalManager::get()->render();

    Debug::init();
    klog('i', "Setting IDT");
    IDT::get()->init();

    klog('i', "Time is %u", CMOS::get()->readTime());

    klog('i', "Configuring timer");
    PIT::get()->init();
    PIT::get()->setFrequency(250);

    Keyboard::get()->init();
    Interrupts::get()->setHandler(IRQ(7),  INTERRUPT_MUTE);
    Interrupts::get()->setHandler(IRQ(15), INTERRUPT_MUTE);
    Interrupts::get()->setHandler(0x00, isrq0);
    Interrupts::get()->setHandler(0x06, isrq6);
    Interrupts::get()->setHandler(0x80, isrq80);
   
    Syscalls::init();
    

    klog('w', "Starting task scheduler");
    Scheduler::get()->init();
    klog_flush();
    Scheduler::get()->spawnKernelThread(&repainterThread, "repainter");
 //   Scheduler::get()->spawnKernelThread(&testThread, "test thread");

    Scheduler::get()->resume();

    // -------------------------------------------------

    auto vfs = VFS::get();
    vfs->mount("/", new FAT32FS());
    vfs->mount("/dev", new DevFS());
    vfs->mount("/proc", new ProcFS());



    FDIR dir;
    f_opendir(&dir, "/root");
    struct dirent* de;
    klog('i', "--");
    for(;;) {
        FILINFO fi;
        char lfnBuffer[255];
        fi.lfname = lfnBuffer;
        fi.lfsize = 255;
        f_readdir(&dir, &fi);
        klog('i', lfnBuffer);
        if (lfnBuffer[0] == 0)
            break;
    }
    klog('i', "--");



    Process* p = Scheduler::get()->spawnProcess(Scheduler::get()->kernelProcess, "a.out");

    PTY* pty = PhysicalTerminalManager::get()->getPTY(0);

    p->pty = pty;
    p->attachFile(pty->openSlave());
    p->attachFile(pty->openSlave());
    p->attachFile(pty->openSlave());

    
    auto elf = new ELF();
    elf->loadFromFile("/bin/sh");
    elf->loadIntoProcess(p);

    char** argv = new char*[3];
    argv[0] = "busybox";
    argv[1] = "sh";
    argv[2] = NULL;
    
    char** envp = new char*[1];
    envp[0] = NULL;

    strcpy(p->cwd, "/root");
    
    klog('w', "Starting Busybox");
    elf->startMainThread(p, argv, envp);
    Debug::tracingOn = true;

    Scheduler::get()->resume();

    for (;;)
        CPU::halt();
}