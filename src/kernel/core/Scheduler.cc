#include <core/Scheduler.h>
#include <core/Process.h>
#include <core/MQ.h>
#include <hardware/pit/PIT.h>
#include <kutil.h>


static void handleSaveKernelState(isrq_registers_t* regs) {
    klog('d', "Saving kernel thread state");
    Scheduler::get()->saveKernelState(regs);
}

void Scheduler::saveKernelState(isrq_registers_t* regs) {
    kernelThread->storeState(regs);
}

static void handleTimer(isrq_registers_t* regs) {
    Scheduler::get()->contextSwitch(regs);
}

static void handleForcedTaskSwitch(isrq_registers_t* regs) {
    Scheduler::get()->contextSwitch(regs);
}


void Scheduler::init() {
    Interrupts::get()->setHandler(0x7f, handleSaveKernelState);
    Interrupts::get()->setHandler(0xff, handleForcedTaskSwitch);

    kernelProcess = new Process();
    kernelProcess->addressSpace = AddressSpace::kernelSpace;
    kernelThread = new Thread(kernelProcess, "Kernel idle thread");
    registerThread(kernelThread);
    activeThread = kernelThread;
    asm volatile("int $0x7f"); // handleSaveKernelState

    PIT::MSG_TIMER.registerConsumer((MessageConsumer)&handleTimer);
}

void Scheduler::registerThread(Thread* t) {
    threads.add(t);
    //t->process->threads.push_back(kernelThread);
}

void Scheduler::spawnKernelThread(threadEntryPoint entry, void* arg, const char* name) {
    Thread* t = new Thread(kernelProcess, name);
    t->state.regs = kernelThread->state.regs;
    t->createStack(0x2000);
    t->pushOnStack((uint64_t)arg);
    t->state.regs.rip = (uint64_t)entry;
    klog('d', "Spawning kernel thread '%s' (entrypoint %lx)", name, entry);
    registerThread(t);
}

void Scheduler::scheduleNextThread() {
    static int index = 0;
    bool thread_ok = false;

    do {
        do {
            index = (index + 1) % threads.capacity;
            nextThread = threads[index];
        } while (!nextThread);

        thread_ok = true;
        if (nextThread->activeWait) {
            if (nextThread->activeWait->isComplete())
                nextThread->stopWaiting();
            else
                thread_ok = false;
        }
    } while (!thread_ok);
}

void Scheduler::scheduleNextThread(Thread* t) {
    nextThread = t;
}

void Scheduler::contextSwitch(isrq_registers_t* regs) {
    if (!nextThread)
        scheduleNextThread();

    activeThread->storeState(regs);

    if (nextThread->process->addressSpace != activeThread->process->addressSpace)
        nextThread->process->addressSpace->activate();

    nextThread->recoverState(regs);

    activeThread = nextThread;
    nextThread = NULL;
}

Thread* Scheduler::getActiveThread() {
    return activeThread;
}

void Scheduler::forceThreadSwitchUserspace(Thread* preferred) {
    if (preferred)
        scheduleNextThread(preferred);
    asm volatile ("int $0xff");
}

void Scheduler::forceThreadSwitchISRQContext(Thread* preferred, isrq_registers_t* regs) {
    if (preferred)
        scheduleNextThread(preferred);
    contextSwitch(regs);
}

  