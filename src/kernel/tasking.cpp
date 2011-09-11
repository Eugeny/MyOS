#include "tasking.h"
#include <memory/Heap.h>
#include <core/Processor.h>
#include <util/LinkedList.h>
#include "kutils.h"
#include <memory/Memory.h>


// The start of the task linked list.

static LinkedList<Task*>* tasks;
static Task* current_task;

// Some externs are needed to access members in paging.c...
extern void alloc_frame(page_t*,int,int);
extern u32int initial_esp;
extern "C" u32int read_eip();


Task::Task() {
    static int pid = 0;
    id = pid++;
    esp = eip = NULL;
}

void move_stack(void *new_stack_start, u32int size);
void initialise_tasking()
{
    asm volatile("cli");

    current_task = new Task();

    current_task->addrSpace = Memory::get()->getCurrentSpace();

    tasks = new LinkedList<Task*>();
    tasks->insertLast(current_task);

    asm volatile("sti");
}


void switch_task()
{
    if (!current_task)
        return;

    u32int eip = Processor::getInstructionPointer();
    if (eip == 0x12345)
        return;

    current_task->eip = eip;
    current_task->esp = Processor::getStackPointer();

    current_task = tasks->remove(0);
    tasks->insertLast(current_task);

    Memory::get()->setAddressSpace( current_task->addrSpace);

    asm volatile("         \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           "
      :: "r"(current_task->eip),
         "r"(current_task->esp),
         "r"(Memory::get()->getCurrentSpace()->dir->physicalAddr));
}

int fork()
{
    Processor::disableInterrupts();

    Task *parent_task = current_task;

    Task *new_task = new Task();
    new_task->addrSpace = Memory::get()->getCurrentSpace()->clone();
    tasks->insertLast(new_task);

    u32int eip = Processor::getInstructionPointer();

    if (current_task == parent_task)
    {
        new_task->esp = Processor::getStackPointer();
        new_task->eip = eip;
        asm volatile("sti");

        return new_task->id;
    }
    else
    {
        return 0;
    }

}

int getpid()
{
    return current_task->id;
}
