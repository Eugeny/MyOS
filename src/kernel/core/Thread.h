#ifndef CORE_THREAD_H
#define CORE_THREAD_H

#include <lang/lang.h>
#include <interrupts/Interrupts.h>


class ThreadState {
public:
    isrq_registers_t regs;
};

class Process;

class Thread {
public:
    Thread(Process* p);
    ~Thread();

    void createStack(uint64_t size);
    void storeState(isrq_registers_t* isrq);
    void recoverState(isrq_registers_t* isrq);

    bool   dead;


    uint64_t id;
    void*    stackBottom;
    uint64_t stackSize;

    Process* process;
    ThreadState state;
private:
};

#endif
