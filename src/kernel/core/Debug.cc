#include <core/Debug.h>
#include <core/MQ.h>
#include <kutil.h>

const char* Debug::MSG_DUMP_REGISTERS = "dump-registers";

void Debug::init() {
    MQ::registerMessage(MSG_DUMP_REGISTERS);
    MQ::registerConsumer(MSG_DUMP_REGISTERS, (MessageConsumer)&Debug::onDumpRegisters);
}

void Debug::onDumpRegisters(isrq_registers_t* regs) {
    klog('w', "Register dump");
    klog('w', "RIP: %016lx   RSP: %016lx", regs->rip, regs->rsp);
    klog('w', "RDI: %016lx   RSI: %016lx", regs->rdi, regs->rsi);
    klog('w', "RAX: %016lx   RBX: %016lx", regs->rax, regs->rbx);
    klog('w', "RCX: %016lx   RDX: %016lx", regs->rcx, regs->rdx);
    klog_flush();
}
