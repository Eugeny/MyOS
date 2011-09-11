#include <core/TaskManager.h>
#include <core/Processor.h>
#include <core/Scheduler.h>
#include <memory/Memory.h>


Task::Task() {
    static int pid = 0;
    id = pid++;
    esp = eip = NULL;
}

TaskManager::TaskManager() {
    tasks = NULL;
}

void TaskManager::init() {
    Processor::disableInterrupts();

    Task* kernel = new Task();
    kernel->addrSpace = Memory::get()->getCurrentSpace();

    tasks = new LinkedList<Task*>();
    tasks->insertLast(kernel);
    currentTask = kernel;
    Scheduler::get()->addTask(kernel);

    Processor::enableInterrupts();
}

#define TASKSWITCH_DUMMY_EIP 0x376

void TaskManager::switchTo(Task* t) {
    if (!tasks)
        return;

    u32int eip = Processor::getInstructionPointer();
    if (eip == TASKSWITCH_DUMMY_EIP) // Magic! We've just switched tasks
        return;

    currentTask->eip = eip;
    currentTask->esp = Processor::getStackPointer();

    currentTask = t;

    Memory::get()->setAddressSpace(currentTask->addrSpace);

    asm volatile("         \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%cr3;       \
      mov %3, %%eax;       \
      sti;                 \
      jmp *%%ecx           "
      :: "r"(currentTask->eip),
         "r"(currentTask->esp),
         "r"(Memory::get()->getCurrentSpace()->dir->physicalAddr),
         "r"(TASKSWITCH_DUMMY_EIP));
}


u32int TaskManager::fork() {
    Processor::disableInterrupts();

    Task *parent_task = currentTask;

    Task *new_task = new Task();
    new_task->addrSpace = Memory::get()->getCurrentSpace()->clone();
    tasks->insertLast(new_task);
    Scheduler::get()->addTask(new_task);

    u32int eip = Processor::getInstructionPointer();

    if (currentTask == parent_task) {
        new_task->esp = Processor::getStackPointer();
        new_task->eip = eip;
        Processor::enableInterrupts();

        return new_task->id;
    }
    else
        return 0;
}

Task* TaskManager::getCurrentTask() {
    return currentTask;
}
