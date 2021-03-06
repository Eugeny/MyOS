#include <core/Thread.h>
#include <core/CPU.h>
#include <kutil.h>
#include <core/Process.h>
#include <string.h>


Thread::Thread(Process* p, const char* name) {
    static int tid = 0;
    id = tid++;
    cycles = 0;
    stackSize = 0;
    dead = false;
    process = p;
    activeWait = NULL;
    this->name = strdup(name);
    state.forked = false;
}

Thread::~Thread() {
    if (activeWait)
        stopWaiting();
    delete name;
}

void Thread::createStack(uint64_t size) {
    stackSize = size;
    stackBottom = process->sbrkStack(size);
    klog('t', "Created stack at %lx", stackBottom);
    state.regs.rsp = (uint64_t)stackBottom - 0x4000;
}

void Thread::createStack(uint64_t bottom, uint64_t size) {
    stackSize = size;
    stackBottom = (void*)bottom;
    process->addressSpace->releaseSpace(bottom-size, size);
    process->allocateStack(bottom-size, size);
    klog('t', "Created stack at %lx", stackBottom);
    state.regs.rsp = (uint64_t)stackBottom - 0x4000;
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

uint64_t Thread::pushOnStack(uint64_t v) {
    state.regs.rsp -= sizeof(uint64_t);
    process->addressSpace->write(&v, state.regs.rsp, 8);
    return state.regs.rsp;
}

uint64_t Thread::pushOnStack(void* buffer, uint64_t size) {
    size = (size + 7) / 8 * 8;
    state.regs.rsp -= size;
    process->addressSpace->write(buffer, state.regs.rsp, size);
    return state.regs.rsp;
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

    if (this == Scheduler::get()->getActiveThread()) {
        Scheduler::get()->resume();
        while (1) {
            CPU::CLI();
            if (!activeWait || activeWait->isComplete())
                break;
            CPU::STI();
            CPU::halt();
        }
    }
}

void Thread::stopWaiting() {
    auto w = activeWait;
    activeWait = NULL;
    delete w;
}