#ifndef SYSCALL_SYSCALLMANAGER_H
#define SYSCALL_SYSCALLMANAGER_H

#include <util/cpp.h>
#include <util/Singleton.h>
#include <interrupts/Interrupts.h>


class SyscallManager : public Singleton<SyscallManager> {
public:
    void init();
    void handleSyscall(isrq_registers_t r);
    void registerSyscall(int cmd, interrupt_handler h);
    void registerDefaults();
private:
    interrupt_handler handlers[256];
};

#endif
