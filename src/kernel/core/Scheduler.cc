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
    __output("TASK OUT", 70);
    Scheduler::get()->contextSwitch(regs);
    __output("TASK IN", 70);
    __outputhex(regs->rip, 60);
}

static void handleForcedTaskSwitch(isrq_registers_t* regs) {
    Scheduler::get()->contextSwitch(regs);
}


void Scheduler::init() {
    Interrupts::get()->setHandler(0x7f, handleSaveKernelState);
    Interrupts::get()->setHandler(0xff, handleForcedTaskSwitch);

    kernelProcess = new Process("Kernel");
    kernelProcess->addressSpace = AddressSpace::kernelSpace;
    kernelProcess->isKernel = true;

    kernelThread = new Thread(kernelProcess, "idle thread");
    kernelThread->state.addressSpace = AddressSpace::kernelSpace;
    kernelProcess->threads.add(kernelThread);
    
    registerThread(kernelThread);

    activeThread = kernelThread;
    
    asm volatile("int $0x7f"); // handleSaveKernelState

    PIT::MSG_TIMER.registerConsumer((MessageConsumer)&handleTimer);

    active = false;
}

void Scheduler::pause() {
    active = false;
}

void Scheduler::resume() {
    active = true;
}

void Scheduler::registerThread(Thread* t) {
    threads.add(t);
}

Process* Scheduler::spawnProcess(const char* name) {
    Process* p = new Process(name);
    p->addressSpace = AddressSpace::kernelSpace->clone();
    return p;
}

Thread* Scheduler::spawnKernelThread(threadEntryPoint entry, const char* name) {
    Thread* t = kernelProcess->spawnThread(entry, name);
    klog('d', "Spawning kernel thread '%s' (entrypoint %lx)", name, entry);
    registerThread(t);
    return t;
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
    if (!active)
        return;
    if (!nextThread)
        scheduleNextThread();

    activeThread->storeState(regs);

    //if (nextThread->process->addressSpace != AddressSpace::current) {
        //activeThread->process->addressSpace = AddressSpace::current;
        //nextThread->process->addressSpace->activate();
    //}

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

  
void Scheduler::logTask() {
    klog('i', "Active task: %s (%i) / %s (%i)", 
        activeThread->process->name, 
        activeThread->process->pid,
        activeThread->name,
        activeThread->id);
}