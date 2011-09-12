#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <util/LinkedList.h>
#include <core/Thread.h>


class Scheduler : public Singleton<Scheduler> {
public:
    Scheduler();
    void  init();
    void  addThread(Thread* t);
    void  removeThread(Thread* t);
    Thread *pickThread();
private:
    LinkedList<Thread*>* queue;
};

#endif
