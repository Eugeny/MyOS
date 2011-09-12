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

void TaskManager::switchTo(Thread* t) {
    if (!t)
        return;

    u32int eip = Processor::getInstructionPointer();
    if (eip == TASKSWITCH_DUMMY_EIP) // Magic! We've just switched tasks
        return;

    currentThread->eip = eip;
    currentThread->esp = Processor::getStackPointer();

    currentThread = t;

    Memory::get()->setAddressSpace(currentThread->process->addrSpace);

    asm volatile("         \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%cr3;       \
      mov %3, %%eax;       \
      sti;                 \
      jmp *%%ecx           "
      :: "r"(currentThread->eip),
         "r"(currentThread->esp),
         "r"(Memory::get()->getCurrentSpace()->dir->physicalAddr),
         "r"(TASKSWITCH_DUMMY_EIP));
}


u32int TaskManager::fork() {
    Processor::disableInterrupts();

    Process* parent = currentThread->process;
    Process* newProc = new Process();
    newProc->name = strclone(parent->name);
    newProc->parent = parent;
    newProc->addrSpace = parent->addrSpace->clone();

    processes->insertLast(newProc);
    parent->children->insertLast(newProc);

    Thread* newThread = new Thread(newProc);
    Scheduler::get()->addThread(newThread);

bool old;

    u32int eip = Processor::getInstructionPointer();

    old = currentThread->process == parent;
    if (old) {
        newThread->esp = Processor::getStackPointer();
        newThread->eip = eip;
        Processor::enableInterrupts();

        return newProc->pid;
    }
    else {
        return 0;
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
    else
        return 0;
}


Thread* TaskManager::getCurrentThread() {
    return currentThread;
}
