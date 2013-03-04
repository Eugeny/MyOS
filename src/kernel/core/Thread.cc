#include <core/Thread.h>
#include <core/CPU.h>
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
    state.fsbase = CPU::RDMSR(MSR_FSBASE);
    state.addressSpace = AddressSpace::current;
}

void Thread::recoverState(isrq_registers_t* regs) {
    *(regs) = state.regs;
    CPU::WRMSR(MSR_FSBASE, state.fsbase);
    state.addressSpace->activate();
}

void Thread::pushOnStack(uint64_t v) {
    AddressSpace* oldAS = AddressSpace::current;
    process->addressSpace->activate();
    state.regs.rsp -= sizeof(uint64_t);
    *((uint64_t*)state.regs.rsp) = v;
    oldAS->activate();
}

void Thread::setEntryArguments(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f) {
    state.regs.rdi = a;
    state.regs.rsi = b;
    state.regs.rdx = c;
    state.regs.rcx = d;
    state.regs.r8 = e;
    state.regs.r9 = f;
}


void Thread::wait(Wait* w) {
    if (activeWait)
        stopWaiting();
    activeWait = w;
}

void Thread::stopWaiting() {
    delete activeWait;
    activeWait = NULL;
}