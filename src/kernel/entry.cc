#include "kutils.h"
#include <core/Processor.h>
#include <core/TaskManager.h>
#include <core/Scheduler.h>
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
#include <vfs/VFS.h>
#include <vfs/Stat.h>


void on_timer(isrq_registers_t r) {
static    Terminal* sb = TTYManager::get()->getStatusBar();
static    int ram = Memory::get()->getUsedFrames() * 100 / Memory::get()->getTotalFrames();
    sb->goTo(WIDTH-10, 0);
    sb->write("RAM: xx%");
    sb->setCh(WIDTH-4, 0, '0'+ram/10);
    sb->setCh(WIDTH-5, 0, '0'+ram%10);

    sb->goTo(WIDTH-22, 0);
    sb->write("Frames ");
    sb->write(to_dec(Memory::get()->getUsedFrames()));

    TTYManager::get()->draw();


    TaskManager::get()->switchTo(Scheduler::get()->pickThread());
}

void kbdh(u32int mod, u32int sc) {
    klogn("K");klogn(to_hex(sc));klogn("-");klog(to_hex(mod));
    TTYManager::get()->processKey(mod, sc);
}


void list(char* p, int d) {
    LinkedList<char*>* l = VFS::get()->listFiles(p);
    LinkedListIter<char*>* i = l->iter();
    for (; !i->end(); i->next()) {
        for (int j=0;j<d;j++)
        klogn(" ");
        klog(i->get());
        klog_flush();

        char* path = (char*)kmalloc(strlen(p)+strlen(i->get()) + 2);
        memcpy(path, p, strlen(p));
        path[strlen(p)] = '/';
        memcpy(path + strlen(p) + 1, i->get(), strlen(i->get())+1);

        if (strcmp(p, "/"))
            memcpy(path + strlen(p), i->get(), strlen(i->get())+1);

        Stat* s = VFS::get()->stat(path);

        if (s && s->isDirectory)
            list(path, d+1);

        delete path;
        delete s;
    }
}

void thread(void* x) {
    DEBUG("Thread!");
    DEBUG((char*)x);
}

#include <hardware/ATA.h>

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


    VFS::get()->init();
    VFS::get()->mount(new RootFS(), "");

    DevFSMaster::get()->init();

    for (int i = 0; i < TTYManager::get()->getTTYCount(); i++)
        DevFSMaster::get()->addTTY(TTYManager::get()->getTTY(i));

    VFS::get()->mount(DevFSMaster::get()->getFS(), "/dev");

// INIT DONE
    FileObject* tty = VFS::get()->open("/dev/tty0", MODE_R|MODE_W);
    ///tty->writeString("Hello!\n");

    u8int buf[512];
    ata_read(0, buf);
    for (int i=0;i<8;i++) {
        u8int b = 0;
        b += buf[i];
        klog(to_hex(b));
    }

    int pid =0;
//    list("/", 0);
//    pid=fork();
  //  pid=fork();
    //pid=fork();
//    newThread(thread,  (void*)"FFFU");

for(;;);


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
