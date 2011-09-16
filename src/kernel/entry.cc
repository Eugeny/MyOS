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
#include <syscall/Syscalls.h>
#include <util/cpp.h>
#include <vfs/DevFS.h>
#include <vfs/RootFS.h>
#include <vfs/FATFS.h>
#include <vfs/VFS.h>
#include <vfs/Stat.h>


void on_timer(isrq_registers_t *r) {
static    Terminal* sb = TTYManager::get()->getStatusBar();
//static    int ram = Memory::get()->getUsedFrames() * 100 / Memory::get()->getTotalFrames();
    sb->goTo(WIDTH-20, 0);
    sb->write("kheap: ");
    sb->write(to_dec(Heap::get()->getUsage()));
    sb->write(" b");

    sb->goTo(WIDTH-32, 0);
    sb->write("Frames ");
    sb->write(to_dec(Memory::get()->getUsedFrames()));

    TTYManager::get()->draw();


    TaskManager::get()->performRoutine();
    TaskManager::get()->nextTask();
}

void kbdh(u32int mod, u32int sc) {
    klogn("K");klogn(to_hex(sc));klogn("-");klog(to_hex(mod));
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

        if (strcmp(p, "/"))
            memcpy(path + strlen(p), i->get(), strlen(i->get())+1);

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

void thread(void* x) {
    DEBUG("Thread!");
}


extern "C" void kmain (void* mbd, u32int esp) {
    initialiseConstructors();

    heap_selfinit();
    Heap::get()->init();

    GDT::get()->init();
    GDT::get()->setDefaults();
    GDT::get()->flush();

    IDT::get()->init();
    Memory::get()->startPaging(esp);

    TTYManager::get()->init(5);
    PIT::get()->setFrequency(50);
    PIT::get()->setHandler(on_timer);

    Keyboard::get()->init();
    Keyboard::get()->setHandler(kbdh);

    Scheduler::get()->init();
    TaskManager::get()->init();

    SyscallManager::get()->init();
    SyscallManager::get()->registerDefaults();


    Disk::get()->init();

    VFS::get()->init();
    VFS::get()->mount(new FATFS(), "");

    DevFSMaster::get()->init();

    for (int i = 0; i < TTYManager::get()->getTTYCount(); i++)
        DevFSMaster::get()->addTTY(TTYManager::get()->getTTY(i));

//    VFS::get()->mount(DevFSMaster::get()->getFS(), "/dev");

// INIT DONE
  //  FileObject* tty = VFS::get()->open("/dev/tty0", MODE_R|MODE_W);
    ///tty->writeString("Hello!\n");


    int pid =0;
//    list("/boot", 0);

    FileObject* f = VFS::get()->open("/app", MODE_R);
    char* ss = (char*)kmalloc(10000);
    f->read(ss, 0, 65536);

    MEMTRACE
    ELF_exec((u8int*)ss);
    MEMTRACE
//    asm volatile ("jmp 0x100");
//    klog(ss);


//    pid=fork();
//    pid=fork();

    //pid=fork();
//    newThread(thread,  (void*)"FFFU");


        char s[] = "> Process x x reporting\n";
        int c = 0;
        int p = TaskManager::get()->getCurrentThread()->id;
        while (1) {
            s[10] = (char)((int)'0' + p%10);
            //klog(to_dec(getpid()));
            //klog(s);klog_flush();
            TTYManager::get()->getTTY(p)->writeString(s);
            for (int i=0;i<100000000;i++);
        }



    for(;;);
}
