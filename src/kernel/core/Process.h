#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <core/Thread.h>
#include <memory/AddressSpace.h>

class Process {
public:
    Process();
    void* requestMemory(u32int sz);

    int pid;
    char* name;
    Process* parent;
    LinkedList<Thread*>* threads;
    LinkedList<Process*>* children;
    AddressSpace* addrSpace;

private:
    u32int allocatorTop;
};
#endif
