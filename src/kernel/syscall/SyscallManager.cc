#include <syscall/SyscallManager.h>
#include <core/TaskManager.h>
#include <core/Thread.h>
#include <kutils.h>


void handler(isrq_registers_t* r) {
    SyscallManager::get()->handleSyscall(r);
}

void SyscallManager::init() {
    memset(handlers, 0, sizeof(interrupt_handler) * 256);
    Interrupts::get()->setHandler(128, handler);
}

void SyscallManager::registerSyscall(int idx, interrupt_handler h) {
    handlers[idx] = h;
}

void SyscallManager::handleSyscall(isrq_registers_t* r) {
    handlers[r->eax](r);
}


void handlePrint(isrq_registers_t* r) {
    klog((char*)r->ebx);
}

void handleFork(isrq_registers_t* r) {
    int pid = TaskManager::get()->fork();
    r->eax = pid;
}

void handleThread(isrq_registers_t* r) {
    thread_entry_point m = (thread_entry_point)r->ebx;
    void* a = (void*)r->ecx;
    int id = TaskManager::get()->newThread(m, a);
    r->eax = id;
}

void handleDie(isrq_registers_t* r) {
    TaskManager::get()->requestKillProcess(TaskManager::get()->getCurrentThread()->process->pid);
}

void SyscallManager::registerDefaults() {
    registerSyscall(0, handlePrint);
    registerSyscall(1, handleFork);
    registerSyscall(2, handleThread);
    registerSyscall(3, handleDie);
}
