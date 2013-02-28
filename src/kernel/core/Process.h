#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <lang/lang.h>
#include <lang/Pool.h>
#include <memory/AddressSpace.h>
#include <interrupts/Interrupts.h>


class Thread;

class Process {
public:
    Process();
    ~Process();

    void* sbrk(uint64_t size);

    uint64_t brk;
    uint64_t pid;
    char* name;

    AddressSpace* addressSpace;
    
    Pool<Thread*, 1024> threads;

private:
};
#endif
