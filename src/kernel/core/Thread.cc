#include <core/Thread.h>
#include <core/Processor.h>
#include <core/Process.h>
#include <core/TaskManager.h>
#include <kutils.h>
#include <memory/Heap.h>


Thread::Thread(Process *p) {
    static int tid = 0;
    id = tid++;
    process = p;
    process->threads->insertLast(this);
    stackSize = 0;
    wait = NULL;
    TSS = (tss_t*)kmalloc_a(sizeof(tss_t));
    memset(TSS, 0, sizeof(tss_t));
    TSS->cr3 = process->addrSpace->dir->physicalAddr;
    TSS->eflags = 2;
    TSS->cs = 0x08;
    TSS->ds = 0x10;
    TSS->es = 0x10;
    TSS->fs = 0x10;
    TSS->gs = 0x10;
    TSS->ss = 0x10;
    TSS->esp = Processor::getStackPointer();
    TSS->esp0 = Processor::getStackPointer();
    TSS->iomapbase = sizeof(tss_t);
    dead = false;
}

Thread::~Thread() {
    delete TSS;
}
