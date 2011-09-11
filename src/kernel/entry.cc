#include "kutils.h"
#include "timer.h"
#include <interrupts/IDT.h>
#include <interrupts/Interrupts.h>
#include <memory/Heap.h>
#include <memory/GDT.h>
#include <memory/Memory.h>
#include <util/cpp.h>
#include "tasking.h"


void on_timer(isrq_registers_t r) {
    static int tick = 0;
    char s[] = "Timer tick 0000";
    s[11] = '0' + tick/1000%10;
    s[12] = '0' + tick/100%10;
    s[13] = '0' + tick/10%10;
    s[14] = '0' + tick%10;
    tick++;
    kprintsp(s, 60, 0);
    switch_task();
    klog_flush();
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

u32int initial_esp;
extern "C" void kmain (void* mbd, u32int esp)
{
    initial_esp = esp;

    heap_selfinit();
    Heap::get()->init();

    klog_init();
    initialiseConstructors();


    Heap::get()->init();

    GDT::get()->init();
    GDT::get()->setDefaults();
    GDT::get()->flush();

    IDT::get()->init();

    Interrupts::get()->setHandler(IRQ(0), on_timer);
    init_timer(5);

    klog("Starting paging");
    Memory::get()->startPaging();

    initialise_tasking();


    int pid = fork();fork();fork();

        char s[] = "> Process x x reporting";
        int c = 0;
        int p = getpid();
        while (1) {
            s[10] = (char)((int)'0' + getpid());
            s[12] = (char)((int)'0' + p);
            klog(s);klog_flush();
            for (int i=0;i<30000000;i++);
        }


    if (pid == 0) {
        char s[] = "Child reporting";
        int c = 0;
        while (1) {
            s[5] = (char)((int)'0' + getpid());
            klog(s);
            for (int i=0;i<100000000;i++);
        }
    } else {
        while (1) {
            klog("Parent reports memory");
 //           mem_dbg();
            for (int i=0;i<100000000;i++);
        }
    }

    /*
    mem_dbg();

    u32int a, b, c, d, e, t;

    a = alloc_dbg("a", 32);

    klog("Enabling heap allocator");
    kheap_enable();


    a = alloc_dbg("a", 32);
    b = alloc_dbg("b", 16);
    c = alloc_dbg("c", 16);
    d = alloc_dbg("d", 16);

    t = *((u32int*)a);

    klogn("free a");
    kfree(a);
    klog(" free c");
    kfree(c);

    e = alloc_dbg("e", 3);
    c = alloc_dbg("c", 40);

    kheap_dbg();

    mem_dbg();
    */


    for(;;);
}
