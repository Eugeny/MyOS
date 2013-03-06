#include <core/Scheduler.h>
#include <core/CPU.h>
#include <core/Process.h>
#include <core/MQ.h>
#include <hardware/pit/PIT.h>
#include <kutil.h>
#include <errno.h>


static Thread* __saving_state_for = NULL;
static void* __saving_state_stack_buf = NULL;
static uint64_t __saving_state_stack_buf_used = 0;
static uint64_t __saving_state_stack_buf_size = 0;


static void handleSaveState(isrq_registers_t* regs) {
    klog('d', "Saving thread state");
    __saving_state_for->storeState(regs);
    if (__saving_state_stack_buf) {
        __saving_state_stack_buf_used = (uint64_t)Scheduler::get()->getActiveThread()->stackBottom - regs->rsp;
        klog('d', "Saving 0x%lx bytes of stack from 0x%lx", __saving_state_stack_buf_used, regs->rsp);
        memcpy(__saving_state_stack_buf, (void*)regs->rsp, __saving_state_stack_buf_used);
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

    kernelProcess = new Process(NULL, "Kernel");
    kernelProcess->addressSpace = AddressSpace::kernelSpace;
    kernelProcess->isKernel = true;
    kernelProcess->pgid = 1000;
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

Process* Scheduler::spawnProcess(Process* parent, const char* name) {
    Process* p = new Process(parent, name);
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
    #define STACKBUF_SIZE 1024*1024
    static uint8_t stackbuf[STACKBUF_SIZE];

    klog('t', "Stack bottom is 0x%lx, RSP is 0x%lx", activeThread->stackBottom, activeThread->state.regs.rsp);
    uint64_t stackbuf_used = saveState(activeThread, stackbuf, STACKBUF_SIZE);
    //asm volatile(".global fork_continue\nfork_continue:");

    if (activeThread->state.forked)
        return NULL;

    Process* p1 = activeThread->process;
    Process* p2 = p1->clone();
    p2->ppid = p1->pid;
    processes.add(p2);

    pause();
    p2->addressSpace = p1->addressSpace->clone();

    AddressSpace* oldAS = AddressSpace::current;

    Thread* nt = new Thread(p2, activeThread->name);
    p2->threads.add(nt);
    threads.add(nt);
    nt->createStack((uint64_t)activeThread->stackBottom, activeThread->stackSize);
    nt->state = activeThread->state;
    nt->state.addressSpace = p2->addressSpace;
    nt->state.forked = true;

    p2->addressSpace->write(stackbuf, nt->state.regs.rsp, stackbuf_used);

    return p2;
}

uint64_t Scheduler::saveState(Thread* t, void* stack_buf, uint64_t stack_buf_size) {
    __saving_state_for = t;
    __saving_state_stack_buf = stack_buf;
    __saving_state_stack_buf_size = stack_buf_size;
    asm volatile("int $0x7f"); // handleSaveKernelState
    CPU::CLI();
    klog('d', "Thread state saved");
    __saving_state_for = NULL;
    return __saving_state_stack_buf_used;
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

    nextThread->cycles++;
    nextThread->recoverState(regs);

    activeThread = nextThread;
    nextThread = NULL;
}

Thread* Scheduler::getActiveThread() {
    return activeThread;
}

Process* Scheduler::getProcess(uint64_t pid) {
    for (Process* p : processes)
        if (p->pid == pid)
            return p;
    seterr(ESRCH);
    return NULL;
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