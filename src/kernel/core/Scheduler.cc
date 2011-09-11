#include <core/Scheduler.h>

void Scheduler::init() {
    queue = new LinkedList<Task*>();
}

void Scheduler::addTask(Task* t) {
    queue->insertLast(t);
}

void Scheduler::removeTask(Task* t) {
    for (LinkedListIter<Task*>* iter = queue->iter(); !iter->end(); iter->next())
        if (iter->get() == t) {
            queue->remove(iter->index);
            return;
        }
}

Task *Scheduler::pickTask() {
    Task *r = queue->remove(0);
    queue->insertLast(r);
    return r;
}
