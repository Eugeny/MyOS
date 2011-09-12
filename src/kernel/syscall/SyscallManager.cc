#include <syscall/SyscallManager.h>
#include <kutils.h>


void handler(isrq_registers_t r) {
    SyscallManager::get()->handleSyscall(r);
}

void SyscallManager::init() {
    memset(handlers, 0, sizeof(interrupt_handler) * 256);
    Interrupts::get()->setHandler(128, handler);
}

void SyscallManager::registerSyscall(int idx, interrupt_handler h) {
    handlers[idx] = h;
}

void SyscallManager::handleSyscall(isrq_registers_t r) {
    handlers[r.eax](r);
}


void handlePrint(isrq_registers_t r) {
    klog((char*)r.ebx);
}

void SyscallManager::registerDefaults() {
    registerSyscall(0, handlePrint);
}
