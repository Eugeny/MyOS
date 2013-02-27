#include <core/Thread.h>
#include <kutil.h>
#include <core/Process.h>

Thread::Thread(Process* p) {
    static int tid = 0;
    id = tid++;
    stackSize = 0;
    dead = false;
    process = p;
}

Thread::~Thread() {
}

void Thread::createStack(uint64_t size) {
    stackSize = size;
    stackBottom = process->sbrk(size);
    state.regs.rsp = (uint64_t)stackBottom + stackSize;
}

void Thread::storeState(isrq_registers_t* regs) {
    state.regs = *(regs);
}

void Thread::recoverState(isrq_registers_t* regs) {
    *(regs) = state.regs;
}