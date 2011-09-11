#ifndef TASKING_H
#define TASKING_H

#include <util/cpp.h>
#include <memory/AddressSpace.h>

// This structure defines a 'task' - a process.
class Task {
public:
    Task();
    u32int id;
    u32int esp;
    u32int eip;
    AddressSpace *addrSpace;
};

// Initialises the tasking system.
void initialise_tasking();

// Called by the timer hook, this changes the running process.
void switch_task();

// Forks the current process, spawning a new one with a different
// memory space.
int fork();


// Returns the pid of the current process.
int getpid();

#endif
