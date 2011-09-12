#ifndef CORE_THREAD_H
#define CORE_THREAD_H

#include <util/cpp.h>

class Process;

typedef struct thread_state {
    u32int eip, esp, ebp, ebx, edi, esi, ph;
} PACKED thread_state_t;


typedef void(*thread_entry_point)(void*);


class Thread {
public:
    Thread(Process *p);
    void die();
    bool isDead();

    u32int id;
    Process *process;
    thread_state_t state;
private:
    bool   dead;
};

#endif
