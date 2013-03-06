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
#include <fcntl.h>

#include <elf/ELF.h>
#include <multiboot.h>



int main() { return 0; }


void pit_handler(isrq_registers_t* regs) {
    static int counter = 0;
    counter++;
    microtrace();
}

void isrq80(isrq_registers_t* regs)  {
    klog('w', "SYSCALL #%x", regs->rax);
    klog_flush();
}

void isrq6(isrq_registers_t* regs)  {
    klog('w', "Invalid opcode, rip=%lx", regs->rip);
    klog_flush();
    for(;;);
}


void repainterThread(void*) {
    for (;;) {
        Scheduler::get()->getActiveThread()->wait(new WaitForDelay(50));
        PhysicalTerminalManager::get()->render();
    }
}

void testThread(void*) {
    PTYSlave* p = PhysicalTerminalManager::get()->openPTY(0);
    for (;;) {
        p->write(".", 1);
        for (int i =0; i < 1000000;i++);
        Scheduler::get()->getActiveThread()->wait(new WaitForDelay(1000));
    }
    for(;;) {
        Scheduler::get()->getActiveThread()->wait(new WaitForDelay(500));
        Scheduler::get()->forceThreadSwitchUserspace(NULL);
        //klog('i', "%i", PIT::get()->getTime());
        //KTRACEMEM
        char buf [1024];
        int c = p->read(buf, 1024);
        p->write(buf, c);
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
    PIT::get()->setFrequency(25);
    PIT::MSG_TIMER.registerConsumer((MessageConsumer)&pit_handler);

    Keyboard::get()->init();
    Interrupts::get()->setHandler(IRQ(7),  INTERRUPT_MUTE);
    Interrupts::get()->setHandler(IRQ(15), INTERRUPT_MUTE);
    Interrupts::get()->setHandler(0x80, isrq80);
    Interrupts::get()->setHandler(0x06, isrq6);


    AddressSpace::kernelSpace->dump();

    //MQ::post(Debug::MSG_DUMP_REGISTERS, NULL);

/*

    Directory* dir = fs->opendir("/");
    struct dirent* de;
    while (de = dir->read()) {
        klog('i', "%s%s", de->d_name, (de->d_type == DT_DIR) ? "/" : "");
    }
    dir->close();
*/

   
    Syscalls::init();
    

    klog('w', "Starting task scheduler");
    Scheduler::get()->init();
    klog_flush();
    Scheduler::get()->spawnKernelThread(&repainterThread, "Screen repainter thread");

    Scheduler::get()->resume();

    // -------------------------------------------------

    auto vfs = VFS::get();
    vfs->mount("/", new FAT32FS());
    vfs->mount("/dev", new DevFS());
    vfs->mount("/proc", new ProcFS());


    auto elf = new ELF();

    Process* p = Scheduler::get()->spawnProcess(Scheduler::get()->kernelProcess, "a.out");
    
    //elf->loadFromFile(vfs->open("/a.out", O_RDONLY));
    elf->loadFromFile(vfs->open("/busybox_unstripped", O_RDONLY));
    //elf->loadFromFile(vfs->open("/dash", O_RDONLY));
    elf->loadIntoProcess(p);
    
    PTY* pty = PhysicalTerminalManager::get()->getPTY(0);

    p->pty = pty;
    p->attachFile(pty->openSlave());
    p->attachFile(pty->openSlave());
    p->attachFile(pty->openSlave());

    p->argc = 2;
    p->argv = new char*[2];
    p->argv[0] = "busybox";
    p->argv[1] = "ash";
    //p->argv[2] = "aash";
    
    p->envc = 2;
    p->env = new char*[2];
    p->env[0] = "USER=root";
    p->env[1] = "PATH=";
    //p->env[2] = NULL;

    p->auxvc = 8;
    p->auxv = new Elf64_auxv_t[9];
    p->setAuxVector(0, AT_PAGESZ, KCFG_PAGE_SIZE);
    p->setAuxVector(1, AT_ENTRY, elf->getEntryPoint());
    p->setAuxVector(2, AT_UID,  0);
    p->setAuxVector(3, AT_GID,  0);
    p->setAuxVector(4, AT_EUID, 0);
    p->setAuxVector(5, AT_EGID, 0);
    p->setAuxVector(6, AT_PLATFORM, (uint64_t)"x86_64");

    p->addressSpace->activate();
    uint64_t randomVec = (uint64_t)p->sbrk(16);
    for (int i = 0; i < 16; i++)
        *(uint8_t*)(randomVec+i) = random() % 256;
    AddressSpace::kernelSpace->activate(); // TODO revert to actual AS

    p->setAuxVector(7, AT_RANDOM, randomVec);
    p->setAuxVector(8, AT_NULL, 0);

    CPU::CLI();
    p->spawnMainThread((threadEntryPoint)elf->getEntryPoint());
    CPU::STI();

    Scheduler::get()->resume();

    for (;;)
        CPU::halt();
}