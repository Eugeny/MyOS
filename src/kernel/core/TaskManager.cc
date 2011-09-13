#include <core/TaskManager.h>
#include <core/Processor.h>
#include <core/Scheduler.h>
#include <core/Process.h>
#include <memory/GDT.h>
#include <memory/Memory.h>
#include <kutils.h>
#include <util/Lock.h>


static Lock* lock;

TaskManager::TaskManager() {
    processes = NULL;
    lock = new Lock();
}


void ltr(u16int selector)
{
   asm ("ltr %0": :"r" (selector));
}

unsigned int str(void)
{
   unsigned int selector;

   asm ("str %0":"=r" (selector));
   return selector;
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

    GDT::get()->setGate(5, (u32int)&(kernel->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);

    Processor::enableInterrupts();
}





void TaskManager::switchTo(Thread* t) {
    if (!t || t == currentThread) {TRACE
        return;}

    if (!lock->attempt()) {TRACE;return;}
//klog("SWITCHING");
//klogn("from: ");klog(to_dec(currentThread->id));
//klogn("to:   ");klog(to_dec(t->id));klog_flush();
    Thread* oldThread = currentThread;
    currentThread = t;
    Memory::get()->setAddressSpace(currentThread->process->addrSpace);

    GDT::get()->setGate(5, (u32int)(oldThread->TSS), sizeof(tss_t), 0x89, 0x40);
    GDT::get()->setGate(6, (u32int)(currentThread->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);

    u32int selector[2];
    selector[0] = 0;
    selector[1] = 0x08*6;

//DEBUG("IN");
    asm volatile ("ljmp %0" :: "m"(*selector));
//DEBUG("OUT");

    lock->release();
//        Memory::get()->getCurrentSpace()->dir->physicalAddr,

}

void dofork() {
    TRACE
    asm volatile ("iret");
}

u32int TaskManager::fork() {
    Processor::disableInterrupts();
    if (!lock->attempt()) return -1;

    Process* parent = currentThread->process;
    Process* newProc = new Process();
    newProc->name = strclone(parent->name);
    newProc->parent = parent;

    processes->insertLast(newProc);
    parent->children->insertLast(newProc);
    newProc->addrSpace = parent->addrSpace->clone();

    Thread* newThread = new Thread(newProc);
    Scheduler::get()->addThread(newThread);

    newThread->TSS->eip = (u32int)&dofork;
    GDT::get()->setGate(5, (u32int)(currentThread->TSS), sizeof(tss_t), 0x89, 0x40);
    GDT::get()->setGate(6, (u32int)(newThread->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);
    static u32int selector[2];
    selector[0] = 0;
    selector[1] = 0x08*6;
    asm volatile ("lcall %0" :: "m"(*selector));
    bool old = TaskManager::get()->getCurrentThread()->process == parent;
    if (!old) {
        lock->release();
        return 0;
    }
    else {
        memcpy(newThread->TSS, currentThread->TSS, sizeof(tss_t));
        newThread->TSS->cr3 = newThread->process->addrSpace->dir->physicalAddr;
        lock->release();
        Processor::enableInterrupts();
        return newProc->pid;
    }
}

static void threadEnd() {for(;;);
    TaskManager::get()->getCurrentThread()->die();
}

Thread *TaskManager::newThread(void (*main)(void*), void* arg) {
    if (!lock->attempt()) {TRACE;return NULL;}
    Processor::disableInterrupts();
TRACE    u32int stack = (u32int)currentThread->process->requestMemory(0x2000) + 0x1FF0;
//u32int stack = Processor::getStackPointer() - 0x2000;
TRACE
    Thread* newThread = new Thread(currentThread->process);
    Scheduler::get()->addThread(newThread);

    newThread->TSS->eip = (u32int)&dofork;
    GDT::get()->setGate(5, (u32int)(currentThread->TSS), sizeof(tss_t), 0x89, 0x40);
    GDT::get()->setGate(6, (u32int)(newThread->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);
    static u32int selector[2];
    selector[0] = 0;
    selector[1] = 0x08*6;
DEBUG("IN");
    asm volatile ("lcall %0" :: "m"(*selector));
DEBUG("OUT");
    bool old = TaskManager::get()->getCurrentThread() != newThread;
    TRACE
    if (!old) {
TRACE
        lock->release();
        Processor::enableInterrupts();
TRACE
        //(*main)(arg);
TRACE
        for(;;);
TRACE
        return 0;
    }
    else {TRACE
        memcpy(newThread->TSS, currentThread->TSS, sizeof(tss_t));
//        newThread->TSS->cr3 = newThread->process->addrSpace->dir->physicalAddr;
        newThread->TSS->esp0 = stack;
        lock->release();
        Processor::enableInterrupts();
        return newThread;
    }
}


Thread* TaskManager::getCurrentThread() {
    return currentThread;
}
