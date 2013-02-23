#include "kutils.h"
#include <core/Processor.h>
#include <core/TaskManager.h>
#include <core/Scheduler.h>
#include <elf/ELF.h>
#include <hardware/Disk.h>
#include <hardware/PIT.h>
#include <hardware/Keyboard.h>
#include <interrupts/IDT.h>
#include <interrupts/Interrupts.h>
#include <io/FileObject.h>
#include <memory/Heap.h>
#include <memory/GDT.h>
#include <memory/Memory.h>
#include <tty/TTYManager.h>
#include <syscall/SyscallManager.h>
#include <util/cpp.h>
#include <vfs/DevFS.h>
#include <vfs/RootFS.h>
#include <vfs/FATFS.h>
#include <vfs/VFS.h>
#include <vfs/Stat.h>


void on_timer(isrq_registers_t *r) {
    TaskManager::get()->nextTask();
    TaskManager::get()->performRoutine();
}

void kbdh(u32int mod, u32int sc) {
    //klogn("K");klogn(to_hex(sc));klogn("-");klog(to_hex(mod));klog_flush();
    if (sc == 0x99 && mod == KBD_MOD_CTRL) {
        tasklist(); return;
    }
    TTYManager::get()->processKey(mod, sc);
}


void list(char* p, int d) {
    LinkedList<char*>* l = VFS::get()->listFiles(p);
    if (!l) return;
    LinkedListIter<char*>* i = l->iter();
    for (; !i->end(); i->next()) {
        for (int j=0;j<d;j++)
            klogn(" ");
        klog(i->get());

        char* path = (char*)kmalloc(strlen(p)+strlen(i->get()) + 2);
        memcpy(path, p, strlen(p));
        path[strlen(p)] = '/';
        memcpy(path + strlen(p) + 1, i->get(), strlen(i->get())+1);

        if (strcmp((char*)p, (char*)"/"))
            memcpy((void*)(path + strlen(p)), i->get(), strlen(i->get())+1);

//        DEBUG(path);
        Stat* s = VFS::get()->stat(path);

        if (s && s->isDirectory)
            list(path, d+1);

        delete s;
        delete path;
    }

    l->purge();
    delete i;
    delete l;
}


void repainterThread(void* a) {
    Terminal* sb = TTYManager::get()->getStatusBar();
    while(true) {
        sb->goTo(WIDTH-20, 0);
        sb->write("kheap: ");
        sb->write(to_dec(Heap::get()->getUsage()));
        sb->write(" b");

        sb->goTo(WIDTH-32, 0);
        sb->write("Frames ");
        sb->write(to_dec(Memory::get()->getUsedFrames()));

        TTYManager::get()->draw();
    }
}


extern "C" void kmain (void* mbd, u32int esp) {
    initialiseConstructors();

    heap_selfinit();
    Heap::get()->init();
    
    IDT::get()->init();

    GDT::get()->init();
    GDT::get()->setDefaults();
    GDT::get()->flush();

    Memory::get()->startPaging(esp);

    TTYManager::get()->init(5);
    klog("MyOS booting");

    klog("Initializing scheduler");
    Scheduler::get()->init();
    TaskManager::get()->init();

    klog("Initializing syscalls");
    SyscallManager::get()->init();
    SyscallManager::get()->registerDefaults();

    klog("Preparing VFS");
    Disk::get()->init();

    VFS::get()->init();
    VFS::get()->mount(new FATFS(), "");

    klog("Mounting /dev");
    DevFSMaster::get()->init();
    for (int i = 0; i < TTYManager::get()->getTTYCount(); i++)
        DevFSMaster::get()->addTTY(TTYManager::get()->getTTY(i));
    VFS::get()->mount(DevFSMaster::get()->getFS(), "/dev");

    TaskManager::get()->newThread(repainterThread, 0);

    PIT::get()->setFrequency(250);
    PIT::get()->setHandler(on_timer);

    Keyboard::get()->init();
    Keyboard::get()->setHandler(kbdh);

    //klogn("Memory barrier:");
    //klog(to_hex(Heap::get()->getFreeSpaceBoundary()));

    FileObject* tty = VFS::get()->open("/dev/tty0", MODE_R|MODE_W);
    klog("Starting init");

    Process::create("/sbin/init", 0,0,tty,tty,tty);

    TaskManager::get()->idle();
    for(;;);
}
