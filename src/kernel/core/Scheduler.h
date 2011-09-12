#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <util/LinkedList.h>
#include <core/TaskManager.h>


class Scheduler : public Singleton<Scheduler> {
public:
    Scheduler();
    void  init();
    void  addTask(Task* t);
    void  removeTask(Task* t);
    Task *pickTask();
private:
    LinkedList<Task*>* queue;
};

#endif
