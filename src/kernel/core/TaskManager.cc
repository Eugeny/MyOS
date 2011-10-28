#include <core/TaskManager.h>
#include <core/Processor.h>
#include <core/Scheduler.h>
#include <core/Process.h>
#include <interrupts/Interrupts.h>
#include <memory/GDT.h>
#include <memory/Memory.h>
#include <kutils.h>
#include <util/Lock.h>


static Lock* lock;

TaskManager::TaskManager() {
    processes = NULL;
    lock = new Lock();
    paused = false;
}


static void ltr(u16int selector)
{
   asm ("ltr %0": :"r" (selector));
}

static unsigned int str(void)
{
   unsigned int selector;

   asm ("str %0":"=r" (selector));
   return selector;
}


void task_fail(isrq_registers_t* r) {
    PANIC("INT6");
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
    kernelProcess = kproc;

    GDT::get()->setGate(5, (u32int)&(kernel->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);

    Interrupts::get()->setHandler(6, task_fail);
    Processor::enableInterrupts();
}


void TaskManager::pause() {
    paused = true;
}

void TaskManager::resume() {
    paused = false;
}


void TaskManager::switchTo(Thread* t) {
    if (!t || t == currentThread) {
        return;}

    if (!lock->attempt()) {TRACE;return;}

    if (TASKSWITCH_DEBUG) {
        klog("SWITCHING");
        klogn("from: ");klog(to_dec(currentThread->id));
        klogn("to:   ");klog(to_dec(t->id));klog_flush();
    }
    
    Thread* oldThread = currentThread;
    currentThread = t;
    Memory::get()->setAddressSpace(currentThread->process->addrSpace);

    GDT::get()->setGate(5, (u32int)(oldThread->TSS), sizeof(tss_t), 0x89, 0x40);
    GDT::get()->setGate(6, (u32int)(currentThread->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);

    u32int selector[2];
    selector[0] = 0;
    selector[1] = 0x08*6;

    asm volatile ("ljmp *%0" :: "m"(*selector));

    lock->release();
//        Memory::get()->getCurrentSpace()->dir->physicalAddr,

}

void dofork() {
    asm volatile ("iret");
}

u32int TaskManager::fork() {
    Processor::disableInterrupts();
    if (!lock->attempt()) return -1;

    Process* parent = currentThread->process;
    Process* newProc = new Process();
    newProc->name = strdup(parent->name);
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
    Processor::enableInterrupts();
    asm volatile ("lcall *%0" :: "m"(*selector));
    bool old = TaskManager::get()->getCurrentThread()->process == parent;
    if (!old) {
        lock->release();
        return 0;
    }
    else {
        memcpy(newThread->TSS, currentThread->TSS, sizeof(tss_t));
        newThread->TSS->cr3 = newThread->process->addrSpace->dir->physicalAddr;
        lock->release();
        return newProc->pid;
    }
}


#define THREADSWITCH_MAGIC 0x84386346
u32int TaskManager::newThread(void (*main)(void*), void* arg) {
    Processor::disableInterrupts();
    if (!lock->attempt()) return -1;

    Process* parent = currentThread->process;
    Process* newProc = new Process();
    newProc->name = strdup(parent->name);
    newProc->parent = parent;

    processes->insertLast(newProc);
    parent->children->insertLast(newProc);
    newProc->addrSpace = parent->addrSpace;//->clone();

    Thread* newThread = new Thread(newProc);
    Scheduler::get()->addThread(newThread);


    u32int stack = (u32int)newThread->process->requestMemory(0x1000);


    newThread->TSS->eip = (u32int)&dofork;
    GDT::get()->setGate(5, (u32int)(currentThread->TSS), sizeof(tss_t), 0x89, 0x40);
    GDT::get()->setGate(6, (u32int)(newThread->TSS), sizeof(tss_t), 0x89, 0x40);
    ltr(0x08*5);
    static u32int selector[2];
    selector[0] = 0;
    selector[1] = 0x08*6;
    asm volatile ("lcall *%0" :: "m"(*selector));

    u32int st;
    asm volatile ("mov 0x4(%%esp), %0" : "=r"(st));

    if (st == THREADSWITCH_MAGIC) {
        lock->release();
        Processor::enableInterrupts();
        asm volatile ("mov 0x8(%%esp), %0" : "=r"(main));
        asm volatile ("mov 0xC(%%esp), %0" : "=r"(arg));
        main(arg);
        TaskManager::get()->requestKillThread(TaskManager::get()->getCurrentThread()->id);
        while (true) Processor::idle();
        return 0;
    }
    else {
        memcpy(newThread->TSS, currentThread->TSS, sizeof(tss_t));
        newThread->TSS->cr3 = newThread->process->addrSpace->dir->physicalAddr;
        newThread->TSS->esp = stack;
        *(u32int*)(stack+4) = THREADSWITCH_MAGIC;
        *(u32int*)(stack+8) = (u32int)main;
        *(u32int*)(stack+12) = (u32int)arg;
        lock->release();
        Processor::enableInterrupts();
        return newProc->pid;
    }
}

void TaskManager::killProcess(Process* p) {
    if (p == currentThread->process) return;

    for (int i = 0; i < p->threads->length(); i++)
        killThread(p->threads->get(i));
    for (int i = 0; i < p->children->length(); i++)
        p->children->get(i)->parent = kernelProcess;
    p->addrSpace->release();
    processes->remove(p);
    delete p->addrSpace;
    delete p;
}

void TaskManager::killThread(Thread* t) {
    if (t == currentThread) return;

    // Free stack
    for (u32int j = t->stackBottom; j < t->stackBottom + t->stackSize; j += 0x1000)
        Memory::get()->free(t->process->addrSpace->getPage(j, false));
    Scheduler::get()->removeThread(t);
    t->process->threads->remove(t);
    if (t->process->threads->length() == 0)
        requestKillProcess(t->process->pid);
    delete t;
    return;
}

void TaskManager::requestKillProcess(u32int pid) {
    for (int i = 0; i < processes->length(); i++)
        if (processes->get(i)->pid == pid) {
            Process* p = processes->get(i);
            p->dead = true;
            if (p == currentThread->process) {
                nextTask();
                while (true) Processor::idle();
            }
        }
}

void TaskManager::requestKillThread(u32int tid) {
    for (int i = 0; i < processes->length(); i++) {
        Process* proc = processes->get(i);
        for (int i = 0; i < proc->threads->length(); i++)
            if (proc->threads->get(i)->id == tid) {
                Thread* t = proc->threads->get(i);
                t->dead = true;
                if (t == currentThread) {
                    nextTask();
                    while (true) Processor::idle();
                }
                return;
            }
    }
}

void TaskManager::nextTask() {
    if (paused)
        return;
        
    switchTo(Scheduler::get()->pickThread());
}

void TaskManager::performRoutine() {
    for (int i = 0; i < processes->length(); i++) {
        Process* proc = processes->get(i);
        for (int j = 0; j < proc->threads->length(); j++)
            if (proc->threads->get(j)->dead) {
                killThread(proc->threads->get(j));
                return;
            }
        if (proc->dead) {
            killProcess(proc);
            return;
        }
    }
}

Thread* TaskManager::getCurrentThread() {
    return currentThread;
}

void TaskManager::idle() {
    TaskManager::get()->performRoutine();
    TaskManager::get()->nextTask();
}