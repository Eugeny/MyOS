#ifndef CORE_TASKMANAGER_H
#define CORE_TASKMANAGER_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <util/Singleton.h>
#include <core/Thread.h>
#include <core/Process.h>


class TaskManager : public Singleton<TaskManager> {
public:
    TaskManager();
    void    init();
    u32int  fork();
    void    switchTo(Thread *t);
    Thread *getCurrentThread();
    Thread *newThread(void (*main)(void*), void* arg);
private:
    LinkedList<Process*>* processes;
    Thread* currentThread;
};

#endif
