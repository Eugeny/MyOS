#include <core/Scheduler.h>

Scheduler::Scheduler() {
    queue = NULL;
}

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
    if (queue->length() == 0)
        return NULL;
    Task *r = queue->remove(0);
    queue->insertLast(r);
    return r;
}
