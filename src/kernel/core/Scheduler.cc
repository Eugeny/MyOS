#include <core/Scheduler.h>
#include <core/CPU.h>
#include <core/Process.h>
#include <core/MQ.h>
#include <hardware/pit/PIT.h>
#include <kutil.h>


static Thread* __saving_state_for = NULL;
static void* __saving_state_stack_buf = NULL;
static uint64_t __saving_state_stack_buf_size = 0;


static void handleSaveState(isrq_registers_t* regs) {
    klog('d', "Saving thread state");
    __saving_state_for->storeState(regs);
    if (__saving_state_stack_buf) {
        klog('d', "Saving stack");
        memcpy(__saving_state_stack_buf, (void*)regs->rsp, __saving_state_stack_buf_size);
    }
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
    Interrupts::get()->setHandler(0x7f, handleSaveState);
    Interrupts::get()->setHandler(0xff, handleForcedTaskSwitch);

    kernelProcess = new Process("Kernel");
    kernelProcess->addressSpace = AddressSpace::kernelSpace;
    kernelProcess->isKernel = true;
    processes.add(kernelProcess);

    kernelThread = new Thread(kernelProcess, "idle thread");
    kernelThread->state.addressSpace = AddressSpace::kernelSpace;
    kernelProcess->threads.add(kernelThread);
    
    registerThread(kernelThread);

    activeThread = kernelThread;
    
    __saving_state_for = kernelThread;
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
    processes.add(p);
    return p;
}

Thread* Scheduler::spawnKernelThread(threadEntryPoint entry, const char* name) {
    klog('d', "Spawning kernel thread '%s' (entrypoint %lx)", name, entry);
    Thread* t = kernelProcess->spawnThread(entry, name);
    registerThread(t);
    return t;
}


extern "C" void fork_continue();

Process* Scheduler::fork() {
    static uint8_t stackbuf[1024];
    uint64_t stackbufSize = (uint64_t)activeThread->stackBottom - activeThread->state.regs.rsp;
    saveState(activeThread, stackbuf, stackbufSize);
    asm volatile(".global fork_continue\nfork_continue:");

    if (activeThread->state.forked)
        return NULL;

KTRACE
    Process* p1 = activeThread->process;
KTRACE
    Process* p2 = p1->clone();
KTRACE
    processes.add(p2);

KTRACE
    p2->addressSpace = p1->addressSpace->clone();
KTRACE

    AddressSpace* oldAS = AddressSpace::current;
KTRACE
    //p1->addressSpace->dump();

    Thread* nt = new Thread(p2, activeThread->name);
    threads.add(nt);
    nt->createStack((uint64_t)activeThread->stackBottom, activeThread->stackSize);
    nt->state = activeThread->state;
    nt->state.addressSpace = p2->addressSpace;
    nt->state.forked = true;

    p2->addressSpace->write(stackbuf, nt->state.regs.rsp, stackbufSize);

    return p2;
}

void Scheduler::saveState(Thread* t, void* stack_buf, uint64_t stack_buf_size) {
    __saving_state_for = t;
    __saving_state_stack_buf = stack_buf;
    __saving_state_stack_buf_size = stack_buf_size;
    KTRACE
    asm volatile("int $0x7f"); // handleSaveKernelState
    CPU::CLI();
    klog('d', "Thread state saved");
    KTRACE
    __saving_state_for = NULL;

    static uint64_t rsp;
    asm volatile ("mov %%rsp, %0" : "=r"(rsp));
    //((uint64_t*)rsp)[5] = (uint64_t)&fork_continue;//0xccccccccccccc;
    dump_stack(rsp, 0);
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