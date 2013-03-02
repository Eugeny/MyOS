#include <core/Process.h>
#include <core/Thread.h>
#include <kutil.h>


Process::Process() {
    brk = 0x400000;
    isKernel = false;
}

void* Process::sbrk(uint64_t size) {
    addressSpace->allocateSpace(brk, size, isKernel ? 0 : PAGEATTR_USER);
    void* result = (void*)brk;
    brk += size;
    return result;
}

Thread* Process::spawnThread(threadEntryPoint entry, void* argument, const char* name) {
    Thread* t = new Thread(this, name);
    t->state = Scheduler::get()->kernelThread->state;
    t->createStack(0x2000);
    t->pushOnStack((uint64_t)argument);
    t->state.regs.rip = (uint64_t)entry;
    threads.add(t);
    Scheduler::get()->registerThread(t);
    return t;
}

Process::~Process() {
}
