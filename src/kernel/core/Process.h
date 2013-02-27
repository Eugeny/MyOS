#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <lang/lang.h>
#include <memory/AddressSpace.h>
#include <interrupts/Interrupts.h>

#include <vector>


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
    
    std::vector<Thread*> threads;

private:
};
#endif
