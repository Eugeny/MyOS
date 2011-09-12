#ifndef CORE_THREAD_H
#define CORE_THREAD_H

#include <util/cpp.h>

class Process;

class Thread {
public:
    Thread(Process *p);
    void die();

    u32int id;
    Process *process;
    char state[256];

    u32int esp;
    u32int eip;
private:
    bool   dead;
};

#endif
