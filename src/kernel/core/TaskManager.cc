#include <core/TaskManager.h>
#include <core/Processor.h>
#include <core/Scheduler.h>
#include <core/Process.h>
#include <memory/Memory.h>
#include <kutils.h>


TaskManager::TaskManager() {
    processes = NULL;
}

void TaskManager::init() {
    Processor::disableInterrupts();

    Process* kproc = new Process();
    kproc->name = "kernel";
    kproc->addrSpace = Memory::get()->getCurrentSpace();
    kproc->parent = NULL;

    Thread* kernel = new Thread(kproc);
    currentThread = kernel;
    Scheduler::get()->addThread(kernel);

    processes = new LinkedList<Process*>();
    processes->insertLast(kproc);

    Processor::enableInterrupts();
}

#define TASKSWITCH_DUMMY_EIP 0x376

extern "C" void CPUSaveState(u32int);
extern "C" void CPURestoreState(u32int, char*);

void TaskManager::switchTo(Thread* t) {
    if (!t)
        return;

    volatile bool switched = false;

    CPUSaveState((u32int)(currentThread->state));
    if (switched) return;

    currentThread = t;
    Memory::get()->setAddressSpace(currentThread->process->addrSpace);

    switched = true;
    CPURestoreState(
        Memory::get()->getCurrentSpace()->dir->physicalAddr,
        currentThread->state
    );
}


u32int TaskManager::fork() {
    Processor::disableInterrupts();

    Process* parent = currentThread->process;
    Process* newProc = new Process();
    newProc->name = strclone(parent->name);
    newProc->parent = parent;

    processes->insertLast(newProc);
    parent->children->insertLast(newProc);

    Thread* newThread = new Thread(newProc);
    Scheduler::get()->addThread(newThread);

    newProc->addrSpace = parent->addrSpace->clone();

    CPUSaveState((u32int)currentThread->state);
    memcpy(newThread->state, currentThread->state, 256);

    bool old = currentThread->process == parent;
    if (!old) {
        return 0;
    }
    else {
        Processor::enableInterrupts();
        return newProc->pid;
    }
}


static void threadEnd() {
    TaskManager::get()->getCurrentThread()->die();
}

Thread *TaskManager::newThread(void (*main)(void*), void* arg) {
    Processor::disableInterrupts();

    u32int stack = (u32int)currentThread->process->requestMemory(0x2000) + 0x1FF0;

    Thread* newThread = new Thread(currentThread->process);

    Scheduler::get()->addThread(newThread);

    if (currentThread != newThread) {
        newThread->esp = stack - 12;
        *((u32int*)(stack-4)) = stack;
        *((u32int*)(stack-8)) = (u32int)(threadEnd);
        *((u32int*)(stack-12)) = (u32int)arg;

        newThread->eip = (u32int)main;
        Processor::enableInterrupts();

        return newThread;
    }
    else {
        return 0;
    }
}


Thread* TaskManager::getCurrentThread() {
    return currentThread;
}
