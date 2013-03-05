#ifndef CORE_THREAD_H
#define CORE_THREAD_H

#include <lang/lang.h>
#include <interrupts/Interrupts.h>
#include <core/Wait.h>
#include <memory/AddressSpace.h>


class ThreadState {
public:
    isrq_registers_t regs;
    uint64_t fsbase, gsbase;
    AddressSpace* addressSpace;
    bool forked;
};

class Process;

class Thread {
public:
    Thread(Process* p, const char* name);
    ~Thread();

    void createStack(uint64_t size);
    void storeState(isrq_registers_t* isrq);
    void recoverState(isrq_registers_t* isrq);
    void pushOnStack(uint64_t v);
    void setEntryArguments(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f);
    void wait(Wait* w);
    void stopWaiting();
    
    bool   dead;
    char*   name;

    uint64_t id;
    void*    stackBottom;
    uint64_t stackSize;

    Process* process;
    ThreadState state;
    Wait* activeWait;
private:
};

#endif
