#include <core/Debug.h>
#include <kutil.h>

Message Debug::MSG_DUMP_REGISTERS("dump-registers");

void Debug::init() {
    MSG_DUMP_REGISTERS.registerConsumer((MessageConsumer)&Debug::onDumpRegisters);
}

void Debug::onDumpRegisters(isrq_registers_t* regs) {
    klog('w', "Register dump");
    klog('w', "RIP: %016lx   RSP: %016lx", regs->rip, regs->rsp);
    klog('w', "RDI: %016lx   RSI: %016lx", regs->rdi, regs->rsi);
    klog('w', "RAX: %016lx   RBX: %016lx", regs->rax, regs->rbx);
    klog('w', "RCX: %016lx   RDX: %016lx", regs->rcx, regs->rdx);
    klog_flush();
}
