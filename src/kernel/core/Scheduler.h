#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

#include <lang/lang.h>
#include <lang/Pool.h>
#include <lang/Singleton.h>
#include <core/Thread.h>
#include <interrupts/Interrupts.h>


typedef void (*threadEntryPoint)(void*);


class Scheduler : public Singleton<Scheduler> {
public:
    void init();
    void pause();
    void resume();
    Process* spawnProcess(Process* parent, const char* name);
    Thread* spawnKernelThread(threadEntryPoint entry, const char* name);
    void registerThread(Thread* t);
    void scheduleNextThread();
    void scheduleNextThread(Thread* t);
    void contextSwitch(isrq_registers_t*);
    
    void forceThreadSwitchUserspace(Thread* preferred);
    void forceThreadSwitchISRQContext(Thread* preferred, isrq_registers_t* regs);
    
    Process* fork();
    uint64_t saveState(Thread* t, void* stack_buf, uint64_t stack_buf_size);

    Thread* getActiveThread();
    Process* getProcess(uint64_t pid);

    void logTask();
    
    Thread* kernelThread;
    Process* kernelProcess;
    Pool<Process*, 1024> processes;
private:
    Thread* nextThread;
    Pool<Thread*, 1024> threads;
    Thread* activeThread;
    bool active;
};

#endif