#ifndef CORE_TASKMANAGER_H
#define CORE_TASKMANAGER_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <util/Singleton.h>
#include <memory/AddressSpace.h>


class Task {
public:
    Task();
    u32int id;
    u32int esp;
    u32int eip;
    AddressSpace *addrSpace;
};


class TaskManager : public Singleton<TaskManager> {
public:
    TaskManager();
    void   init();
    u32int fork();
    void   switchTo(Task *t);
    Task  *getCurrentTask();
private:
    LinkedList<Task*>* tasks;
    Task* currentTask;
};

#endif
