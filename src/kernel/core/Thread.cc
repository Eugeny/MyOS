#include <core/Thread.h>
#include <kutil.h>
#include <core/Process.h>
#include <string.h>


Thread::Thread(Process* p, const char* name) {
    static int tid = 0;
    id = tid++;
    stackSize = 0;
    dead = false;
    process = p;
    activeWait = NULL;
    this->name = strdup(name);
}

Thread::~Thread() {
    delete name;
}

void Thread::createStack(uint64_t size) {
    stackSize = size;
    stackBottom = process->sbrk(size);
    state.regs.rsp = (uint64_t)stackBottom + stackSize - 16;
}

void Thread::storeState(isrq_registers_t* regs) {
    state.regs = *(regs);
}

void Thread::recoverState(isrq_registers_t* regs) {
    *(regs) = state.regs;
}

void Thread::pushOnStack(uint64_t v) {
    AddressSpace* oldAS = AddressSpace::current;
    process->addressSpace->activate();
    state.regs.rsp -= sizeof(uint64_t);
    *((uint64_t*)state.regs.rsp) = v;
    oldAS->activate();
}

void Thread::wait(Wait* w) {
    activeWait = w;
}

void Thread::stopWaiting() {
    delete activeWait;
    activeWait = NULL;
}