#include <core/Process.h>
#include <memory/Heap.h>


Process::Process() {
    static int id = 0;
    pid = id++;
    threads = new LinkedList<Thread*>();
    children = new LinkedList<Process*>();
    allocatorTop = Heap::get()->getFreeSpaceBoundary();
}

void *Process::requestMemory(u32int sz) {
    void* res = (void*)allocatorTop;
    for (u32int i = allocatorTop; i < allocatorTop + sz+1; i += 0x1000)
        addrSpace->allocatePage(i, true, false, true);
    allocatorTop += sz;
    return res;
}
