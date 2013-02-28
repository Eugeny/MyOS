#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

#include <lang/lang.h>
#include <lang/Singleton.h>
#include <core/Thread.h>
#include <vector>


class Scheduler : public Singleton<Scheduler> {
public:
    void init();
    void addThread(Thread* t);
    void removeThread(Thread* t);
private:
    vector<Thread*> threads;
};

#endif