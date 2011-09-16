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
    u32int  newThread(void (*main)(void*), void* arg);
    void    killThread(Thread* tid);
    void    killProcess(Process* pid);
    void    requestKillThread(u32int tid);
    void    requestKillProcess(u32int pid);

    void    nextTask();
    void    performRoutine();
private:
    LinkedList<Process*>* processes;
    Thread* currentThread;
    Process* kernelProcess;
};

#endif
