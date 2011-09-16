#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <core/Thread.h>
#include <memory/AddressSpace.h>
#include <io/FileObject.h>


class Process {
public:
    Process();
    ~Process();
    void* requestMemory(u32int sz);

    u32int pid;
    char* name;
    bool dead;
    FileObject* files[256];
    Process* parent;
    LinkedList<Thread*>* threads;
    LinkedList<Process*>* children;
    AddressSpace* addrSpace;

private:
    u32int allocatorTop;
};
#endif
