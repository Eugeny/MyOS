#include "kutils.h"
#include <core/TaskManager.h>
#include <core/Scheduler.h>
#include <hardware/PIT.h>
#include <hardware/Keyboard.h>
#include <interrupts/IDT.h>
#include <interrupts/Interrupts.h>
#include <memory/Heap.h>
#include <memory/GDT.h>
#include <memory/Memory.h>
#include <tty/TTYManager.h>
#include <util/cpp.h>


void on_timer(isrq_registers_t r) {
    TaskManager::get()->switchTo(Scheduler::get()->pickTask());

    Terminal* sb = TTYManager::get()->getStatusBar();
    int ram = Memory::get()->getUsedFrames() * 100 / Memory::get()->getTotalFrames();
    sb->goTo(WIDTH-10, 0);
    sb->write("RAM: xx%");
    sb->setCh(WIDTH-4, 0, '0'+ram/10);
    sb->setCh(WIDTH-5, 0, '0'+ram%10);

    sb->goTo(WIDTH-22, 0);
    sb->write("Frames ");
    sb->write(to_dec(Memory::get()->getUsedFrames()));

    TTYManager::get()->draw();
}


u32int alloc_dbg(char* n, u32int sz) {
    u32int r, phy;
    r = (u32int)kmalloc_p(sz, &phy);
    klogn("malloc(");
    klogn(to_dec(sz));
    klogn(") -> ");
    klogn(n);
    klogn(" = virt=0x");
    klogn(to_hex(r));
    klogn(" phy=0x");
    klog(to_hex(phy));
    return r;
}

extern heap_t kheap;
/*void kheap_dbg() {
    klog("\nKernel heap index");
    for (int i=0;i<kheap.index_length;i++) {
        klogn(to_dec(i));
        if (kheap.index[i].flags == HEAP_HOLE)
        klogn(" HOLE ");
        else klogn(" DATA ");
        klogn(to_hex(kheap.index[i].addr));
        klogn(" + ");
        klog(to_hex(kheap.index[i].size));
    }
}

void mem_dbg() {
    memory_info_t meminfo;
    paging_info(&meminfo);
    klogn("\nTotal frames: ");  klog(to_dec(meminfo.total_frames));
    klogn("Used frames:  ");  klog(to_dec(meminfo.used_frames));
}

*/

void kbdh(u32int mod, u32int sc) {
    klogn("K");klogn(to_hex(sc));klogn("-");klog(to_hex(mod));
    TTYManager::get()->processKey(mod, sc);
}

extern "C" void kmain (void* mbd, u32int esp)
{
    initialiseConstructors();

    heap_selfinit();
    Heap::get()->init();

    GDT::get()->init();
    GDT::get()->setDefaults();
    GDT::get()->flush();

    IDT::get()->init();

    Memory::get()->startPaging(esp);

    TTYManager::get()->init(5);

    PIT::get()->setHandler(on_timer);
    PIT::get()->setFrequency(50);

    Keyboard::get()->init();
    Keyboard::get()->setHandler(kbdh);

    Scheduler::get()->init();
    TaskManager::get()->init();

// INIT DONE
    TaskManager::get()->fork();
    TaskManager::get()->fork();

        char s[] = "> Process x x reporting\n";
        int c = 0;
        int p = TaskManager::get()->getCurrentTask()->id;
        while (1) {
            s[10] = (char)((int)'0' + p%10);
            //klog(to_dec(getpid()));
            //klog(s);klog_flush();
            TTYManager::get()->getTTY(p)->writeString(s);
            for (int i=0;i<300000000;i++);
        }

    for(;;);
}
